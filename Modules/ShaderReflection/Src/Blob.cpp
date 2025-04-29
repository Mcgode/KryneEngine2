/**
 * @file
 * @author Max Godefroy
 * @date 14/04/2025.
 */

#include <KryneEngine/Core/Common/StringHelpers.hpp>

#include "KryneEngine/Modules/ShaderReflection/Blob.hpp"

namespace KryneEngine::Modules::ShaderReflection
{
    u32 Blob::GetEntryPointOffset(u32 _index) const
    {
        const u32* entryPointIndirectionTable = reinterpret_cast<const u32*>(this + 1);
        return entryPointIndirectionTable[_index];
    }

    const Blob::EntryPointHeader* Blob::GetEntryPointHeader(u32 _index) const
    {
        return reinterpret_cast<const EntryPointHeader*>(GetByteBuffer() + GetEntryPointOffset(_index));
    }

    bool Blob::IsShaderReflectionBlob(const std::byte* _data)
    {
        return *reinterpret_cast<const u32*>(_data) == kMagicNumber;
    }

    Blob* Blob::CreateBlob(
        AllocatorInstance _allocator,
        eastl::span<const EntryPointInput> _entryPoints,
        size_t& _blobSize)
    {
        // Account for header
        u32 estimatedPreStringTotal = sizeof(Header);

        // Don't forget to count for extra u8 for string size
        u32 stringTotal = 0;

        constexpr auto getStringSize = [](const eastl::string_view& _string)
        {
            return eastl::min<u32>(_string.size(), kMaxStringLength) + 1u;
        };

        // Count entry point indirection table
        estimatedPreStringTotal += Alignment::AlignUp(_entryPoints.size() * sizeof(u32), alignof(Blob));

        for (const auto& entryPoint : _entryPoints)
        {
            // Count entry point header size
            estimatedPreStringTotal += Alignment::AlignUp(sizeof(EntryPointHeader), alignof(Blob));

            // Append entry point name
            stringTotal += getStringSize(entryPoint.m_name);

            // If applicable, save space for push constant name
            stringTotal += entryPoint.m_pushConstants.has_value() ? getStringSize(entryPoint.m_pushConstants->m_name) : 0;

            // Count descriptor set indirection table
            estimatedPreStringTotal += Alignment::AlignUp(entryPoint.m_descriptorSets.size() * sizeof(u32), alignof(Blob));

            for (const auto& descriptorSet : entryPoint.m_descriptorSets)
            {
                estimatedPreStringTotal += sizeof(DescriptorSetHeader);
                stringTotal += getStringSize(descriptorSet.m_name);

                estimatedPreStringTotal += Alignment::AlignUp(sizeof(DescriptorData) * descriptorSet.m_descriptors.size(), alignof(Blob));

                for (const auto& descriptor : descriptorSet.m_descriptors)
                {
                    stringTotal += getStringSize(descriptor.m_name);
                }
            }
        }

        // Allocate blob with the correct size from the get-go
        _blobSize = estimatedPreStringTotal + stringTotal;
        Blob* blob = reinterpret_cast<Blob*>(_allocator.allocate(_blobSize));

        // Fill in the blob header
        blob->m_header.m_magic = kMagicNumber;
        blob->m_header.m_version = kVersion;
        blob->m_header.m_entryPointsCount = _entryPoints.size();
        blob->m_header.m_stringsOffset = estimatedPreStringTotal;

        std::byte* dataIt = nullptr; // Will keep track of the next non-string data save location

        u8* stringIt = reinterpret_cast<u8*>(blob) + estimatedPreStringTotal;
        const auto registerName = [&](eastl::string_view _name)
        {
            const u8 length = eastl::min<u32>(_name.size(), kMaxStringLength);

            KE_ASSERT_MSG(
                stringIt + length + 1 - reinterpret_cast<const u8*>(blob) <= (_blobSize),
                "Out of string space!");

            memcpy(stringIt + 1, _name.data(), length);
            *stringIt = length;
            const u32 offset = stringIt - reinterpret_cast<const u8*>(blob);
            stringIt += length + 1;
            return offset;
        };

        // Fill in entry points blobs first
        {
            u32* entryPointIndirectionIt = reinterpret_cast<u32*>(blob + 1);

            dataIt = reinterpret_cast<std::byte*>(entryPointIndirectionIt) + Alignment::AlignUp(sizeof(u32) * _entryPoints.size(), alignof(Blob));

            for (const auto& entryPoint: _entryPoints)
            {
                *entryPointIndirectionIt = dataIt - blob->GetByteBuffer();
                entryPointIndirectionIt++;

                auto* entryPointHeader = reinterpret_cast<EntryPointHeader*>(dataIt);
                dataIt += sizeof(EntryPointHeader) + Alignment::AlignUp(sizeof(u32) * entryPoint.m_descriptorSets.size(), alignof(Blob));

                entryPointHeader->m_nameHash = StringHash::Hash64(entryPoint.m_name);
                entryPointHeader->m_nameOffset = registerName(entryPoint.m_name);
                entryPointHeader->m_stage = static_cast<u16>(entryPoint.m_stage);
                entryPointHeader->m_descriptorSetCount = entryPoint.m_descriptorSets.size();

                if (entryPoint.m_pushConstants.has_value())
                {
                    entryPointHeader->m_pushConstantsSignatureHash = Hashing::Hash64Append(
                        entryPoint.m_pushConstants->m_size,
                        StringHash::Hash64(entryPoint.m_pushConstants->m_name));
                    entryPointHeader->m_pushConstantsNameOffset = registerName(entryPoint.m_pushConstants->m_name);
                    entryPointHeader->m_pushConstantsByteSize = entryPoint.m_pushConstants->m_size;
                }
                else
                {
                    entryPointHeader->m_pushConstantsSignatureHash = 0;
                    entryPointHeader->m_pushConstantsNameOffset = 0;
                    entryPointHeader->m_pushConstantsByteSize = 0;
                }
            }
        }

        // Fill in descriptor set blobs
        for (auto entryPointIdx = 0u; entryPointIdx < blob->GetEntryPointCount(); entryPointIdx++)
        {
            const u32 entryPointOffset = blob->GetEntryPointOffset(entryPointIdx);
            auto* descriptorSetIndirectionIt = reinterpret_cast<u32*>(blob->GetByteBuffer() + entryPointOffset + sizeof(EntryPointHeader));

            for (const auto& descriptorSet : _entryPoints[entryPointIdx].m_descriptorSets)
            {
                *descriptorSetIndirectionIt = dataIt - blob->GetByteBuffer();
                descriptorSetIndirectionIt++;

                auto* descriptorSetHeader = reinterpret_cast<DescriptorSetHeader*>(dataIt);
                dataIt += sizeof(DescriptorSetHeader);

                descriptorSetHeader->m_signatureHash = StringHash::Hash64(descriptorSet.m_name);
                descriptorSetHeader->m_nameOffset = registerName(descriptorSet.m_name);
                descriptorSetHeader->m_descriptorCount = descriptorSet.m_descriptors.size();

                auto* descriptorIt = reinterpret_cast<DescriptorData*>(dataIt);
                dataIt += Alignment::AlignUp(sizeof(DescriptorData) * descriptorSet.m_descriptors.size(), alignof(Blob));

                for (const auto& descriptor : descriptorSet.m_descriptors)
                {
                    descriptorIt->m_nameOffset = registerName(descriptor.m_name);

                    descriptorSetHeader->m_signatureHash = Hashing::Hash64Append(
                        descriptor.m_name.data(),
                        descriptor.m_name.size(),
                        descriptorSetHeader->m_signatureHash);

                    descriptorIt->m_count = descriptor.m_count;
                    descriptorIt->m_bindingIndex = descriptor.m_bindingIndex;
                    descriptorIt->m_type = descriptor.m_type;
                    descriptorIt->m_textureType = descriptor.m_textureType;

                    descriptorSetHeader->m_signatureHash = Hashing::Hash64Append(
                        reinterpret_cast<const std::byte*>(&descriptorIt->m_count),
                        sizeof(descriptorIt->m_count) + sizeof(descriptorIt->m_bindingIndex) + sizeof(descriptorIt->m_type) + sizeof(descriptorIt->m_textureType),
                        descriptorSetHeader->m_signatureHash);

                    descriptorIt++;
                }
            }
        }

        KE_ASSERT_MSG(stringIt == (reinterpret_cast<u8*>(blob) + _blobSize), "Unused extra padding");

        return blob;
    }
}