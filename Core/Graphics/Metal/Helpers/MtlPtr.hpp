/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#pragma once

#include <Foundation/NSObject.hpp>
#include <concepts>

namespace KryneEngine
{
    template<class T>
    requires std::derived_from<T, NS::Object>
    class MtlPtr
    {
    public:
        MtlPtr(): m_ptr(nullptr) {}
        explicit MtlPtr(T* _ptr) : m_ptr(_ptr) {}

        ~MtlPtr()
        {
            if (m_ptr != nullptr)
            {
                m_ptr->release();
            }
        }

        MtlPtr& operator=(T* _ptr)
        {
            reset(_ptr);
            return *this;
        }

        T* operator->() const { return m_ptr; }
        T& operator*() const { return *m_ptr; }

        T* get() const { return m_ptr; }

        void reset(T* _ptr = nullptr)
        {
            if (m_ptr != nullptr)
            {
                m_ptr->release();
            }

            m_ptr = _ptr;

            if (m_ptr != nullptr)
            {
                m_ptr->retain();
            }
        }

    private:
        T* m_ptr;
    };
}