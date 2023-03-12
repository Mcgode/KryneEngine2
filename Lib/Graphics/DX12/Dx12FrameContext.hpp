/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#pragma once

#include <Threads/LightweightMutex.hpp>
#include "Dx12Headers.hpp"

namespace KryneEngine
{
    class Dx12FrameContext
    {
    public:
        Dx12FrameContext(ID3D12Device* _device, bool _directAllocator, bool _computeAllocator, bool _copyAllocator);

        virtual ~Dx12FrameContext();

        ID3D12GraphicsCommandList* BeginDirectCommandList()
        {
            return m_directCommandAllocationSet.BeginCommandList(m_device.Get(), D3D12_COMMAND_LIST_TYPE_DIRECT);
        }

        void EndDirectCommandList()
        {
            m_directCommandAllocationSet.EndCommandList();
        }

        ID3D12GraphicsCommandList* BeginComputeCommandList()
        {
            return m_computeCommandAllocationSet.BeginCommandList(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COMPUTE);
        }

        void EndComputeCommandList()
        {
            m_computeCommandAllocationSet.EndCommandList();
        }

        ID3D12GraphicsCommandList* BeginTransferCommandList()
        {
            return m_copyCommandAllocationSet.BeginCommandList(m_device.Get(), D3D12_COMMAND_LIST_TYPE_COPY);
        }

        void EndTransferCommandList()
        {
            m_copyCommandAllocationSet.EndCommandList();
        }

    private:
        ComPtr<ID3D12Device> m_device;

        struct CommandAllocationSet
        {
            ComPtr<ID3D12CommandAllocator> m_commandAllocator = nullptr;

            eastl::vector<ID3D12GraphicsCommandList*> m_availableCommandLists;
            eastl::vector<ID3D12GraphicsCommandList*> m_usedCommandLists;

            LightweightMutex m_mutex {};

            ID3D12GraphicsCommandList *BeginCommandList(ID3D12Device *_device, D3D12_COMMAND_LIST_TYPE _commandType);
            void EndCommandList();

            void Destroy();
        };

        CommandAllocationSet m_directCommandAllocationSet;
        CommandAllocationSet m_computeCommandAllocationSet;
        CommandAllocationSet m_copyCommandAllocationSet;
    };
} // KryneEngine