/**
 * @file
 * @author Max Godefroy
 * @date 14/04/2025.
 */

#pragma once

#include <KryneEngine/Core/Common/Types.hpp>

#include "KryneEngine/Modules/ShaderReflection/Input/EntryPoint.hpp"

namespace KryneEngine::Modules::ShaderReflection
{
    /**
     * @brief A binary blob containing the shader reflection data for a specific shader
     *
     * @details
     * The blob is formatted as such:
     *      - header
     *      - entry point indirection table
     *      - entry point blobs
     *      - descriptor set blobs
     *      - strings table
     *
     * Entry points are kept in contiguous memory, as they are expected to be accessed in sequence often, mostly by
     * iterating over them to verify if one of them matches a given entry point name.
     *
     * Since entry points are kept in contiguous memory, descriptor set blobs are kept in a separate part of the file.
     * The descriptor set blobs are regrouped by entry point for optimal cache hit with most likely access pattern.
     * Each descriptor set header is followed by its descriptor list for the same reason. More often than not we expect
     * to access a descriptor set in its entirety rather than iterating of the set headers.
     *
     * Strings are kept in a separate table, as most of them may not be accessed at all, and could thus take up some
     * cache space for nothing.
     */
    struct alignas(sizeof(u64)) Blob
    {
        static constexpr size_t kAlignment = sizeof(u64);
        static constexpr u32 kMagicNumber = 0x91eb21ad; // 'keshrf' in base64
        static constexpr u32 kVersion = 0;
        static constexpr u32 kMaxStringLength = 255;

        struct Header
        {
            u32 m_magic;
            u32 m_version;
            u32 m_entryPointsCount;
            u32 m_stringsOffset;
        } m_header;

        struct EntryPointHeader
        {
            u64 m_nameHash;

            u32 m_nameOffset;
            u16 m_stage;
            u16 m_descriptorSetCount;

            u64 m_pushConstantsSignatureHash;
            u32 m_pushConstantsNameOffset;
            u32 m_pushConstantsByteSize;
        };

        struct DescriptorSetHeader
        {
            u64 m_signatureHash;
            u32 m_nameOffset;
            u32 m_descriptorCount;
        };

        struct DescriptorData
        {
            u32 m_nameOffset;
            u16 m_count;
            u16 m_bindingIndex;
            DescriptorBindingDesc::Type m_type;
            TextureTypes m_textureType;
        };

        [[nodiscard]] const std::byte* GetByteBuffer() const { return reinterpret_cast<const std::byte*>(this); }
        [[nodiscard]] std::byte* GetByteBuffer() { return reinterpret_cast<std::byte*>(this); }

        [[nodiscard]] u32 GetEntryPointCount() const { return m_header.m_entryPointsCount; }
        [[nodiscard]] u32 GetEntryPointOffset(u32 _index) const;
        [[nodiscard]] const EntryPointHeader* GetEntryPointHeader(u32 _index) const;

        [[nodiscard]] static bool IsShaderReflectionBlob(const std::byte* _data);
        [[nodiscard]] static Blob* CreateBlob(
            AllocatorInstance _allocator,
            eastl::span<const EntryPointInput> _entryPoints);
    };
}
