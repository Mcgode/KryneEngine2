/**
 * @file
 * @author Max Godefroy
 * @date 03/11/2024.
 */

#include "MetalArgumentBufferManager.hpp"

#include <Graphics/Common/ShaderPipeline.hpp>
#include <Graphics/Metal/Helpers/EnumConverters.hpp>
#include <Graphics/Metal/MetalHeaders.hpp>
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    union PackedIndex
    {
        static constexpr size_t kTypeBits = 8;

        struct {
            u32 m_type: kTypeBits;
            u32 m_index: 32 - kTypeBits;
        };
        u32 m_packedIndex;
    };

    MetalArgumentBufferManager::MetalArgumentBufferManager() = default;
    MetalArgumentBufferManager::~MetalArgumentBufferManager() = default;

    void MetalArgumentBufferManager::Init(u8 _inFlightFrameCount)
    {
        m_inFlightFrameCount = _inFlightFrameCount;
    }

    DescriptorSetLayoutHandle MetalArgumentBufferManager::CreateArgumentDescriptor(
        const DescriptorSetDesc& _desc,
        u32* _bindingIndices)
    {
        const GenPool::Handle handle = m_argumentDescriptors.Allocate();
        ArgumentDescriptorHotData* hot = m_argumentDescriptors.Get(handle);

        hot->m_argDescriptors.Resize(_desc.m_bindings.size());
        hot->m_argDescriptors.InitAll(nullptr);

        for (size_t i = 0; i < hot->m_argDescriptors.Size(); i++)
        {
            NsPtr<MTL::ArgumentDescriptor>& desc = hot->m_argDescriptors[i];
            const DescriptorBindingDesc& binding = _desc.m_bindings[i];

            desc = MTL::ArgumentDescriptor::alloc()->init();
            desc->setDataType(MetalConverters::GetDataType(binding.m_type));
            desc->setAccess(MetalConverters::GetBindingAccess(binding.m_type));
            desc->setArrayLength(binding.m_count);
            desc->setIndex(i);
            desc->setTextureType(MetalConverters::GetTextureType(binding.m_textureType));
            _bindingIndices[i] = PackedIndex {
                .m_type = static_cast<u32>(binding.m_type),
                .m_index = static_cast<u32>(i),
            }.m_packedIndex;
        }

        return { handle };
    }

    bool MetalArgumentBufferManager::DeleteArgumentDescriptor(DescriptorSetLayoutHandle _argDescriptor)
    {
        ArgumentDescriptorHotData hot;
        if (m_argumentDescriptors.Free(_argDescriptor.m_handle, &hot))
        {
            hot.m_argDescriptors.Clear();
            return true;
        }
        return false;
    }
} // namespace KryneEngine