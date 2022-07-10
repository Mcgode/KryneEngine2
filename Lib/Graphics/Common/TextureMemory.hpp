/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#pragma once

#include <Common/KETypes.hpp>
#include <atomic>

namespace KryneEngine
{
    class TextureMemory
    {
        friend class TextureView;

    public:
        [[nodiscard]] virtual u64 GetMemorySize() const = 0;

    protected:
        explicit TextureMemory(bool _manualDestroy);

        virtual ~TextureMemory();

        virtual void _Destroy() = 0;

        /// @brief A function to call in the derived class before reaching the base class destructor.
        void _BeforeDestruct();

        void _AddView() { m_userCount++; }
        void _RemoveView() { m_userCount--; }

    protected:
        const bool m_manualDestroy;

    private:
        std::atomic<s32> m_userCount;
        mutable bool m_beforeDestructCalled = false;
    };
}
