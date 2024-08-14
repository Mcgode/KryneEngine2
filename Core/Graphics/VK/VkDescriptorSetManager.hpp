/**
 * @file
 * @author Max Godefroy
 * @date 13/08/2024.
 */

#pragma once

#include "VkHeaders.hpp"
#include <Graphics/Common/Handles.hpp>

namespace KryneEngine
{
    struct DescriptorSetDesc;

    class VkDescriptorSetManager
    {
    public:
        VkDescriptorSetManager();
        ~VkDescriptorSetManager();

        void Init(u8 _frameCount);

        [[nodiscard]] DescriptorSetLayoutHandle CreateDescriptorSetLayout(
            const DescriptorSetDesc& _desc,
            u32* _bindingIndices,
            VkDevice _device);
        bool DestroyDescriptorSetLayout(DescriptorSetLayoutHandle _layout, VkDevice _device);

        [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout(DescriptorSetLayoutHandle _layout);

        [[nodiscard]] DescriptorSetHandle CreateDescriptorSet(DescriptorSetLayoutHandle _layout, VkDevice _device);
        bool DestroyDescriptorSet(DescriptorSetHandle _descriptorSet, VkDevice _device);

    private:
        u64 m_frameCount = 0;

        struct LayoutData
        {
            VkDescriptorSetLayout m_layout;
            eastl::vector<VkDescriptorPoolSize> m_poolSizes;
        };
        GenerationalPool<LayoutData> m_descriptorSetLayouts;

        GenerationalPool<VkDescriptorPool> m_descriptorSetPools;
        eastl::vector<VkDescriptorSet> m_descriptorSets;
    };
} // namespace KryneEngine