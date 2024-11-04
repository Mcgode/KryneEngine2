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
    class ArgumentEncoder;
    class Buffer;
    class Device;
}

namespace KryneEngine
{
    struct DescriptorSetDesc;

    class MetalArgumentBufferManager
    {
    public:
        MetalArgumentBufferManager();
        ~MetalArgumentBufferManager();

        void Init(u8 _inFlightFrameCount);

    private:
        u8 m_inFlightFrameCount;

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

    public:
        [[nodiscard]] DescriptorSetHandle
        CreateArgumentBuffer(MTL::Device& _device, DescriptorSetLayoutHandle _descriptor);
        bool DestroyArgumentBuffer(DescriptorSetHandle _argumentBuffer);

    private:
        struct ArgumentBufferHotData
        {
            NsPtr<MTL::ArgumentEncoder> m_encoder;
            NsPtr<MTL::Buffer> m_argumentBuffer;
        };

        GenerationalPool<ArgumentBufferHotData> m_argumentBufferSets;
    };
} // namespace KryneEngine
