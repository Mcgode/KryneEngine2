/**
 * @file
 * @author Max Godefroy
 * @date 13/08/2024.
 */

#pragma once

#include "Graphics/Vulkan/VkHeaders.hpp"
#include "KryneEngine/Core/Common/Utils/MultiFrameTracking.hpp"
#include "KryneEngine/Core/Graphics/Common/Handles.hpp"
#include "KryneEngine/Core/Graphics/Common/ShaderPipeline.hpp"

namespace KryneEngine
{
    struct DescriptorSetDesc;
    struct VkResources;

    class VkDescriptorSetManager
    {
    public:
        friend class VkGraphicsContext;

        VkDescriptorSetManager();
        ~VkDescriptorSetManager();

        void Init(u8 _frameCount, u8 _frameIndex);

        [[nodiscard]] DescriptorSetLayoutHandle CreateDescriptorSetLayout(
            const DescriptorSetDesc& _desc,
            u32* _bindingIndices,
            VkDevice _device);
        bool DestroyDescriptorSetLayout(DescriptorSetLayoutHandle _layout, VkDevice _device);

        [[nodiscard]] VkDescriptorSetLayout GetDescriptorSetLayout(DescriptorSetLayoutHandle _layout);

        [[nodiscard]] DescriptorSetHandle CreateDescriptorSet(DescriptorSetLayoutHandle _layout, VkDevice _device);
        bool DestroyDescriptorSet(DescriptorSetHandle _descriptorSet, VkDevice _device);

        void UpdateDescriptorSet(
            DescriptorSetHandle _descriptorSet,
            const eastl::span<const DescriptorSetWriteInfo>& _writes,
            VkDevice _device,
            const VkResources& _resources,
            u8 _frameIndex);

        void NextFrame(VkDevice _device, const VkResources& _resources, u8 _frameIndex);

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

        struct WriteOp
        {
            DescriptorSetHandle m_descriptorSet;
            DescriptorSetWriteInfo m_info;
        };

        MultiFrameDataTracker<WriteOp> m_multiFrameTracker;

        eastl::vector<WriteOp> m_tmpWriteOps;
        eastl::vector<VkWriteDescriptorSet> m_tmpWrites;

        union DescriptorData
        {
            static_assert(sizeof(VkDescriptorImageInfo) == sizeof(VkDescriptorBufferInfo), "Types must take full size");

            VkDescriptorImageInfo m_imageInfo;
            VkDescriptorBufferInfo m_bufferImageInfo;
        };
        eastl::vector<DescriptorData> m_tmpDescriptorData;

        void _ProcessUpdates(
            const eastl::vector<WriteOp>& _writes,
            VkDevice _device,
            const VkResources& _resources,
            u8 _frameIndex);
    };
} // namespace KryneEngine