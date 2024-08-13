/**
 * @file
 * @author Max Godefroy
 * @date 13/08/2024.
 */

#include "HelperFunctions.hpp"
#include "VkDescriptorSetManager.hpp"
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
        const GenPool::Handle handle = m_descriptorSetLayouts.Allocate();
        VkAssert(vkCreateDescriptorSetLayout(
            _device,
            &createInfo,
            nullptr,
            m_descriptorSetLayouts.Get(handle)));
        return { handle };
    }
} // namespace KryneEngine