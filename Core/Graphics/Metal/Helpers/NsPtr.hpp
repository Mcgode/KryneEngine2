/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#pragma once

#include <Foundation/NSObject.hpp>
#include <concepts>

#define KE_AUTO_RELEASE_POOL NsPtr autoReleasePool { NS::AutoreleasePool::alloc()->init() }

namespace KryneEngine
{
    template<class T>
    class NsPtr
    {
    public:
        NsPtr(): m_ptr(nullptr) {}
        explicit NsPtr(T* _ptr) : m_ptr(_ptr) {}

        NsPtr& operator=(T* _ptr)
        {
            reset(_ptr);
            return *this;
        }

        NsPtr(const NsPtr& _other)
            : NsPtr(_other.m_ptr)
        {
            if (m_ptr != nullptr)
            {
                m_ptr->retain();
            }
        }

        NsPtr& operator=(const NsPtr& _other)
        {
            reset(_other.m_ptr);
            if (m_ptr)
            {
                m_ptr->retain();
            }
            return *this;
        }

        ~NsPtr()
        {
            if (m_ptr != nullptr)
            {
                m_ptr->release();
            }
        }

        T* operator->() const { return m_ptr; }
        T& operator*() const { return *m_ptr; }

        bool operator==(NsPtr _other) { return m_ptr == _other.m_ptr; }
        bool operator==(T* _other) { return m_ptr == _other; }
        bool operator==(nullptr_t) { return m_ptr == nullptr; }

        T* get() const { return m_ptr; }

        void reset(T* _ptr = nullptr)
        {
            if (m_ptr != nullptr)
            {
                m_ptr->release();
            }

            m_ptr = _ptr;
        }

    private:
        T* m_ptr;
    };
}