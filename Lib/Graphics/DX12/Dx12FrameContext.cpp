/**
 * @file
 * @author Max Godefroy
 * @date 12/03/2023.
 */

#include <Common/Assert.hpp>
#include "Dx12FrameContext.hpp"
#include "HelperFunctions.hpp"

namespace KryneEngine
{
    Dx12FrameContext::Dx12FrameContext(ID3D12Device *_device, bool _directAllocator, bool _computeAllocator, bool _copyAllocator)
    {
        m_device = _device;

        const auto initAllocator = [this] (bool _alloc,CommandAllocationSet& _set, D3D12_COMMAND_LIST_TYPE _type, const wchar_t* _name)
        {
            if (_alloc)
            {
                Dx12Assert(m_device->CreateCommandAllocator(_type, IID_PPV_ARGS(&_set.m_commandAllocator)));

#if !defined(KE_FINAL)
                Dx12SetName(_set.m_commandAllocator.Get(), L"%s Command Allocator", _name);
#endif
            }
        };

        initAllocator(_directAllocator, m_directCommandAllocationSet, D3D12_COMMAND_LIST_TYPE_DIRECT, L"Direct");
        initAllocator(_computeAllocator, m_computeCommandAllocationSet, D3D12_COMMAND_LIST_TYPE_COMPUTE, L"Compute");
        initAllocator(_copyAllocator, m_copyCommandAllocationSet, D3D12_COMMAND_LIST_TYPE_COPY, L"Copy");
    }

    Dx12FrameContext::~Dx12FrameContext()
    {
        m_directCommandAllocationSet.Destroy();
        m_computeCommandAllocationSet.Destroy();
        m_copyCommandAllocationSet.Destroy();
    }

    ID3D12GraphicsCommandList *Dx12FrameContext::CommandAllocationSet::BeginCommandList(ID3D12Device *_device,
                                                                                        D3D12_COMMAND_LIST_TYPE _commandType)
    {
        VERIFY_OR_RETURN(m_commandAllocator != nullptr, nullptr);

        m_mutex.ManualLock();

        if (m_availableCommandLists.empty())
        {
            Dx12Assert(_device->CreateCommandList(0,
                                                  _commandType,
                                                  m_commandAllocator.Get(),
                                                  nullptr,
                                                  IID_PPV_ARGS(&m_usedCommandLists.push_back())));
        }
        else
        {
            m_usedCommandLists.push_back(m_availableCommandLists.back());
            m_availableCommandLists.pop_back();
            Dx12Assert(m_usedCommandLists.back()->Reset(m_commandAllocator.Get(), nullptr));
        }

        return m_usedCommandLists.back();
    }

    void Dx12FrameContext::CommandAllocationSet::EndCommandList()
    {
        VERIFY_OR_RETURN_VOID(m_commandAllocator != nullptr);

        Dx12Assert(m_usedCommandLists.back()->Close());

        m_mutex.ManualUnlock();
    }

    void Dx12FrameContext::CommandAllocationSet::Destroy()
    {
        const auto lock = m_mutex.AutoLock();
        KE_ASSERT_MSG(m_usedCommandLists.empty(), "Allocation set should have been reset");

        const auto freeCommandListVector = [](eastl::vector<ID3D12GraphicsCommandList*>& _vector)
        {
            for (auto commandList: _vector)
            {
                SafeRelease(commandList);
            }
            _vector.clear();
        };
        freeCommandListVector(m_usedCommandLists);
        freeCommandListVector(m_availableCommandLists);

        SafeRelease(m_commandAllocator);
    }
} // KryneEngine