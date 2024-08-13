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

    private:
        u64 m_frameCount = 0;
        GenerationalPool<VkDescriptorSetLayout> m_descriptorSetLayouts;
        GenerationalPool<VkDescriptorPool> m_descriptorSetPools;
        eastl::vector<VkDescriptorSet> m_descriptorSets;
    };
} // namespace KryneEngine