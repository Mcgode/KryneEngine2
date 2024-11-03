/**
 * @file
 * @author Max Godefroy
 * @date 03/11/2024.
 */

#pragma once

#include <Graphics/Common/Handles.hpp>
#include <Graphics/Metal/Helpers/NsPtr.hpp>
#include <Memory/GenerationalPool.hpp>

namespace MTL
{
    class ArgumentDescriptor;
}

namespace KryneEngine
{
    struct DescriptorSetDesc;

    class MetalArgumentBufferManager
    {
    public:
        MetalArgumentBufferManager();
        ~MetalArgumentBufferManager();

    public:
        [[nodiscard]] DescriptorSetLayoutHandle CreateArgumentDescriptor(
            const DescriptorSetDesc& _desc,
            u32* _bindingIndices);
        bool DeleteArgumentDescriptor(DescriptorSetLayoutHandle _argDescriptor);

    private:
        struct ArgumentDescriptorHotData
        {
            DynamicArray<NsPtr<MTL::ArgumentDescriptor>> m_argDescriptors;
        };

        GenerationalPool<ArgumentDescriptorHotData> m_argumentDescriptors;
    };
} // namespace KryneEngine
