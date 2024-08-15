/**
 * @file
 * @author Max Godefroy
 * @date 13/08/2024.
 */

#include "VkDescriptorSetManager.hpp"
#include "HelperFunctions.hpp"
#include "VkResources.hpp"
#include <EASTL/vector_map.h>
#include <Graphics/Common/ShaderPipeline.hpp>
#include <Memory/GenerationalPool.inl>

namespace KryneEngine
{
    union PackedIndex
    {
        static constexpr u8 kTypeBits = 10;

        struct {
            u32 m_type : kTypeBits;
            u32 m_binding : 32 - kTypeBits;
        };
        u32 m_packed;
    };

    VkDescriptorSetManager::VkDescriptorSetManager() = default;
    VkDescriptorSetManager::~VkDescriptorSetManager() = default;

    void VkDescriptorSetManager::Init(u8 _frameCount, u8 _frameIndex)
    {
        m_frameCount = _frameCount;
        m_multiFrameTracker.Init(_frameCount, _frameIndex);
    }

    DescriptorSetLayoutHandle VkDescriptorSetManager::CreateDescriptorSetLayout(
        const DescriptorSetDesc& _desc,
        u32* _bindingIndices,
        VkDevice _device)
    {
        eastl::vector<VkDescriptorSetLayoutBinding> bindings;
        bindings.reserve(_desc.m_bindings.size());

        eastl::vector_map<VkDescriptorType, u32> countPerType;

        for (auto i = 0u; i < _desc.m_bindings.size(); i++)
        {
            const DescriptorBindingDesc& binding = _desc.m_bindings[i];
            const VkDescriptorType type = VkHelperFunctions::ToVkDescriptorType(binding.m_type);
            bindings.push_back(VkDescriptorSetLayoutBinding {
                .binding = i,
                .descriptorType = type,
                .descriptorCount = binding.m_count,
                .stageFlags = VkHelperFunctions::ToVkShaderStageFlags(binding.m_visibility),
                .pImmutableSamplers = nullptr,
            });
            KE_ASSERT(type < (1 << PackedIndex::kTypeBits));

            countPerType[type] += binding.m_count;

            const PackedIndex packedIndex {
                .m_type = static_cast<u32>(type),
                .m_binding = i,
            };
            _bindingIndices[i] = packedIndex.m_packed;
        }

        const VkDescriptorSetLayoutCreateInfo createInfo {
            .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = static_cast<u32>(bindings.size()),
            .pBindings = bindings.data(),
        };

        VkDescriptorSetLayout layout;
        VkAssert(vkCreateDescriptorSetLayout(
            _device,
            &createInfo,
            nullptr,
            &layout));

        const GenPool::Handle handle = m_descriptorSetLayouts.Allocate();
        auto* data = m_descriptorSetLayouts.Get(handle);
        data->m_layout = layout;
        data->m_poolSizes.clear();
        for (auto [type, count]: countPerType)
        {
            data->m_poolSizes.push_back(VkDescriptorPoolSize { type, static_cast<u32>(count * m_frameCount) });
        }

        return { handle };
    }

    bool VkDescriptorSetManager::DestroyDescriptorSetLayout(DescriptorSetLayoutHandle _layout, VkDevice _device)
    {
        LayoutData data;
        if (m_descriptorSetLayouts.Free(_layout.m_handle, &data))
        {
            vkDestroyDescriptorSetLayout(_device, data.m_layout, nullptr);
            return true;
        }
        return false;
    }

    VkDescriptorSetLayout VkDescriptorSetManager::GetDescriptorSetLayout(DescriptorSetLayoutHandle _layout)
    {
        const LayoutData* pData = m_descriptorSetLayouts.Get(_layout.m_handle);
        VERIFY_OR_RETURN(pData != nullptr, VK_NULL_HANDLE);
        return pData->m_layout;
    }

    DescriptorSetHandle VkDescriptorSetManager::CreateDescriptorSet(DescriptorSetLayoutHandle _layout, VkDevice _device)
    {
        VERIFY_OR_RETURN(_layout != GenPool::kInvalidHandle, {GenPool::kInvalidHandle });
        LayoutData* pLayoutData = m_descriptorSetLayouts.Get(_layout.m_handle);
        VERIFY_OR_RETURN(pLayoutData != nullptr, {GenPool::kInvalidHandle });

        // Create descriptor pool
        VkDescriptorPool pool;
        {
            VkDescriptorPoolCreateInfo createInfo {
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .flags = 0,
                .maxSets = static_cast<u32>(m_frameCount),
                .poolSizeCount = static_cast<u32>(pLayoutData->m_poolSizes.size()),
                .pPoolSizes = pLayoutData->m_poolSizes.data(),
            };

            VkAssert(vkCreateDescriptorPool(
                _device,
                &createInfo,
                nullptr,
                &pool));
        }

        const GenPool::Handle handle = m_descriptorSetPools.Allocate();
        *m_descriptorSetPools.Get(handle) = pool;

        // Allocate descriptor sets
        {
            const u64 offset = static_cast<u64>(handle.m_index) * m_frameCount;
            if (m_descriptorSets.size() < offset + m_frameCount)
            {
                m_descriptorSets.resize(offset + m_frameCount, VK_NULL_HANDLE);
            }

            DynamicArray<VkDescriptorSetLayout> layouts(m_frameCount);
            layouts.InitAll(pLayoutData->m_layout);

            const VkDescriptorSetAllocateInfo allocateInfo{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
                .descriptorPool = pool,
                .descriptorSetCount = static_cast<u32>(m_frameCount),
                .pSetLayouts = layouts.Data(),
            };
            VkAssert(vkAllocateDescriptorSets(
                _device,
                &allocateInfo,
                m_descriptorSets.begin() + offset));
        }

        return { handle };
    }

    bool VkDescriptorSetManager::DestroyDescriptorSet(DescriptorSetHandle _descriptorSet, VkDevice _device)
    {
        VkDescriptorPool pool;
        if (m_descriptorSetPools.Free(_descriptorSet.m_handle, &pool))
        {
            // Descriptor sets in the vector can be left as loose memory, their memory location will be reallocated
            vkDestroyDescriptorPool(_device, pool, nullptr);
            return true;
        }
        return false;
    }

    void VkDescriptorSetManager::UpdateDescriptorSet(
        DescriptorSetHandle _descriptorSet,
        const eastl::span<DescriptorSetWriteInfo>& _writes,
        VkDevice _device,
        const VkResources& _resources,
        u8 _frameIndex)
    {
        m_tmpWriteOps.clear();

        for (const auto& write: _writes)
        {
            WriteOp& writeOp = m_tmpWriteOps.emplace_back();
            writeOp.m_descriptorSet = _descriptorSet;
            writeOp.m_info = write;
            m_multiFrameTracker.TrackForOtherFrames(writeOp);
        }

        _ProcessUpdates(m_tmpWriteOps, _device, _resources, _frameIndex);
    }

    void VkDescriptorSetManager::NextFrame(VkDevice _device, const VkResources& _resources, u8 _frameIndex)
    {
        m_multiFrameTracker.AdvanceToNextFrame();

        _ProcessUpdates(m_multiFrameTracker.GetData(), _device, _resources, _frameIndex);

        m_multiFrameTracker.ClearData();
    }

    void VkDescriptorSetManager::_ProcessUpdates(
        const eastl::vector<WriteOp>& _writes,
        VkDevice _device,
        const VkResources& _resources,
        u8 _frameIndex)
    {
        m_tmpWrites.clear();
        m_tmpWrites.reserve(_writes.size());

        m_tmpDescriptorData.clear();

        VkDescriptorSet set = VK_NULL_HANDLE;
        GenPool::Handle lastSet = GenPool::kInvalidHandle;
        for (const auto writeOp: _writes)
        {
            if (lastSet != writeOp.m_descriptorSet.m_handle)
            {
                lastSet = writeOp.m_descriptorSet.m_handle;
                if (m_descriptorSetPools.Get(lastSet) != nullptr)
                {
                    const u64 index = m_frameCount * lastSet.m_index + _frameIndex;
                    set = m_descriptorSets[index];
                }
                else
                {
                    set = VK_NULL_HANDLE;
                }
            }

            if (set == VK_NULL_HANDLE)
            {
                continue;
            }

            const PackedIndex packedIndex = { .m_packed = writeOp.m_info.m_index };
            const bool isImageInfo = packedIndex.m_type <= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;

            m_tmpWrites.push_back(VkWriteDescriptorSet {
                .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
                // We save the vector offset as it may grow during the operation
                .pNext = reinterpret_cast<void*>(m_tmpDescriptorData.size()),
                .dstSet = set,
                .dstBinding = packedIndex.m_binding,
                .dstArrayElement = writeOp.m_info.m_arrayOffset,
                .descriptorCount = static_cast<u32>(writeOp.m_info.m_descriptorData.size()),
                .descriptorType = static_cast<VkDescriptorType>(packedIndex.m_type),
            });

            for (const auto& descriptor : writeOp.m_info.m_descriptorData)
            {
                auto& data = m_tmpDescriptorData.emplace_back();

                if (packedIndex.m_type == VK_DESCRIPTOR_TYPE_SAMPLER)
                {
                    const VkSampler* pSampler = _resources.m_samplers.Get(descriptor.m_handle);
                    VERIFY_OR_RETURN_VOID(pSampler != nullptr);
                    data.m_imageInfo.sampler = *pSampler;
                }
                else if (isImageInfo)
                {
                    const VkImageView* pImageView = _resources.m_imageViews.Get(descriptor.m_handle);
                    VERIFY_OR_RETURN_VOID(pImageView != nullptr);
                    data.m_imageInfo.imageView = *pImageView;
                    data.m_imageInfo.imageLayout = VkHelperFunctions::ToVkLayout(descriptor.m_textureLayout);
                }
                else
                {
                    KE_ERROR("Buffer support not yet implemented");
                    return;
                }
            }

        }

        // Set the definitive pointers
        for (auto& write: m_tmpWrites)
        {
            const bool isImageInfo = write.descriptorType <= VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
            const auto offset = reinterpret_cast<size_t>(write.pNext);
            write.pNext = nullptr;
            if (isImageInfo)
            {
                write.pImageInfo = &m_tmpDescriptorData[offset].m_imageInfo;
            }
            else
            {
                write.pBufferInfo = &m_tmpDescriptorData[offset].m_bufferImageInfo;
            }
        }

        vkUpdateDescriptorSets(
            _device,
            m_tmpWrites.size(),
            m_tmpWrites.data(),
            0,
            nullptr);
    }
} // namespace KryneEngine