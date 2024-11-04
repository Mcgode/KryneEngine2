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
        auto [hot, cold] = m_argumentDescriptors.GetAll(handle);

        hot->m_argDescriptors.Resize(_desc.m_bindings.size());
        hot->m_argDescriptors.InitAll(nullptr);

        cold->m_shaderVisibility = ShaderVisibility::None;

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

            cold->m_shaderVisibility |= binding.m_visibility;
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

    DescriptorSetHandle MetalArgumentBufferManager::CreateArgumentBuffer(
        MTL::Device& _device,
        DescriptorSetLayoutHandle _descriptor)
    {
        const GenPool::Handle handle = m_argumentBufferSets.Allocate();

        ArgumentDescriptorHotData* argDescHot = m_argumentDescriptors.Get(_descriptor.m_handle);
        ArgumentBufferHotData* hot = m_argumentBufferSets.Get(handle);

        NS::Array* array = NS::Array::array(
            reinterpret_cast<const NS::Object* const*>(argDescHot->m_argDescriptors.Data()),
            argDescHot->m_argDescriptors.Size());
        hot->m_encoder = _device.newArgumentEncoder(array);

#if defined(TARGET_OS_MAC)
        const MTL::ResourceOptions options = MTL::ResourceStorageModeManaged;
#else
        const MTL::ResourceOptions options = MTL::ResourceStorageModeShared;
#endif
        hot->m_argumentBuffer = _device.newBuffer(hot->m_encoder->encodedLength() * m_inFlightFrameCount, options);

        return { handle };
    }

    bool MetalArgumentBufferManager::DestroyArgumentBuffer(DescriptorSetHandle _argumentBuffer)
    {
        ArgumentBufferHotData hot;
        if (m_argumentBufferSets.Free(_argumentBuffer.m_handle, &hot))
        {
            hot.m_encoder.reset();
            hot.m_argumentBuffer.reset();
            return true;
        }
        return false;
    }

    PipelineLayoutHandle MetalArgumentBufferManager::CreatePipelineLayout(const PipelineLayoutDesc& _desc)
    {
        const GenPool::Handle handle = m_pipelineLayouts.Allocate();

        PipelineLayoutHotData* hot = m_pipelineLayouts.Get(handle);

        constexpr ShaderVisibility testedVisibilities[] = {
            ShaderVisibility::Vertex,
            ShaderVisibility::TesselationControl,
            ShaderVisibility::TesselationEvaluation,
            ShaderVisibility::Fragment,
            ShaderVisibility::Compute,
            ShaderVisibility::Mesh,
            ShaderVisibility::Task
        };

        // Reproduce SpirV cross behaviour regarding push constant buffer index determination.
        // If no descriptor set is included in shader, takes buffer index 0.
        // If there's any set, will take the last set index, and add +1.
        // Push constant buffer index can vary between stages.

        for (auto pushConstantDesc : _desc.m_pushConstants)
        {
            auto& data = hot->m_pushConstantsData.emplace_back();
            for (const ShaderVisibility visibility: testedVisibilities)
            {
                if (BitUtils::EnumHasAny(pushConstantDesc.m_visibility, visibility))
                {
                    data.m_data.push_back({
                        .m_visibility = visibility,
                    });
                }
            }
        }

        for (size_t i = 0; i < _desc.m_descriptorSets.size(); i++)
        {
            const auto& set = _desc.m_descriptorSets[i];
            const ShaderVisibility setVisibility = m_argumentDescriptors.GetCold(set.m_handle)->m_shaderVisibility;
            hot->m_setVisibilities.push_back();

            for (auto& pcData: hot->m_pushConstantsData)
            {
                for (auto& visibilityData: pcData.m_data)
                {
                    if (BitUtils::EnumHasAny(setVisibility, visibilityData.m_visibility))
                    {
                        visibilityData.m_bufferIndex = i + 1;
                    }
                }
            }
        }

        return { handle };
    }

    bool MetalArgumentBufferManager::DestroyPipelineLayout(PipelineLayoutHandle _layout)
    {
        return m_pipelineLayouts.Free(_layout.m_handle);
    }
} // namespace KryneEngine