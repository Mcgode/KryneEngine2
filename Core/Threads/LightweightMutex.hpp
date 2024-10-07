/**
 * @file
 * @author Max Godefroy
 * @date 03/07/2022.
 */

#pragma once

#include <mutex>
#include <Threads/SpinLock.hpp>
#include <EASTL/algorithm.h>
#include <client/TracyLock.hpp>

namespace KryneEngine
{
    struct LightweightMutex
    {
    public:
        explicit LightweightMutex(u32 _spinCount = 4'096);

        void ManualLock();

        void ManualUnlock();

        inline void Mark(const tracy::SourceLocationData* _srcLocation) { m_ctx.Mark(_srcLocation); }

        inline void CustomName(const eastl::string_view& _name)
        {
            m_ctx.CustomName(_name.data(), _name.size());
        }

    private:
        SpinLock m_spinLock;
        u32 m_spinCount;
        std::mutex m_systemMutex;
        tracy::LockableCtx m_ctx;

    public:
        using LockGuardT = Threads::SyncLockGuard<LightweightMutex, &LightweightMutex::ManualLock, &LightweightMutex::ManualUnlock>;

        [[nodiscard]] LockGuardT AutoLock()
        {
            return LockGuardT(this);
        }
    };
}