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

    DescriptorSetHandle
    MetalArgumentBufferManager::CreateArgumentBuffer(MTL::Device& _device, DescriptorSetLayoutHandle _descriptor)
    {
        const GenPool::Handle handle = m_argumentBufferSets.Allocate();

        ArgumentDescriptorHotData* argDescHot = m_argumentDescriptors.Get(_descriptor.m_handle);
        ArgumentBufferHotData* hot = m_argumentBufferSets.Get(handle);

        NS::Array* array = NS::Array::array(
            reinterpret_cast<const NS::Object* const*>(argDescHot->m_argDescriptors.Data()),
            argDescHot->m_argDescriptors.Size());
        hot->m_encoder = _device.newArgumentEncoder(array);

        hot->m_argumentBuffers.Resize(m_inFlightFrameCount);
#if defined(TARGET_OS_MAC)
        const MTL::ResourceOptions options = MTL::ResourceStorageModeManaged;
#else
        const MTL::ResourceOptions options = MTL::ResourceStorageModeShared;
#endif
        for (u8 i = 0; i < m_inFlightFrameCount; i++)
        {
            hot->m_argumentBuffers.Init(i, _device.newBuffer(hot->m_encoder->encodedLength(), options));
        }

        return { handle };
    }

    bool MetalArgumentBufferManager::DestroyArgumentBuffer(DescriptorSetHandle _argumentBuffer)
    {
        ArgumentBufferHotData hot;
        if (m_argumentBufferSets.Free(_argumentBuffer.m_handle, &hot))
        {
            hot.m_encoder.reset();
            hot.m_argumentBuffers.Clear();
            return true;
        }
        return false;
    }
} // namespace KryneEngine