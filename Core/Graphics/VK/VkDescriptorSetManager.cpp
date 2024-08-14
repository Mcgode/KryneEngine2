/**
 * @file
 * @author Max Godefroy
 * @date 13/08/2024.
 */

#include "VkDescriptorSetManager.hpp"
#include "HelperFunctions.hpp"
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

    void VkDescriptorSetManager::Init(u8 _frameCount)
    {
        m_frameCount = _frameCount;
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
} // namespace KryneEngine