/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#pragma once

#include <Common/Types.hpp>
#include <Common/Assert.hpp>
#include <EASTL/initializer_list.h>

namespace KryneEngine
{
    template<class T>
    struct DynamicArray
    {
        using Ptr = T*;
        using Ref = T&;
        using Iterator = Ptr;

        DynamicArray() = default;

        explicit DynamicArray(u64 _count)
        {
            Resize(_count);
        }

        DynamicArray(u64 _count, const T& _value)
        {
            Resize(_count);
            SetAll(_value);
        }

        DynamicArray(const std::initializer_list<T>& _initializerList)
        {
            Resize(_initializerList.size());

            auto it = _initializerList.begin();
            for (Ptr valuePtr = begin(); valuePtr != end(); valuePtr++, it++)
            {
                *valuePtr = *it;
            }
        }

        DynamicArray(const DynamicArray& _other)
        {
            Resize(_other.m_count);
            memcpy(m_array, _other.m_array, sizeof(T) * m_count);
        }

        DynamicArray& operator=(const DynamicArray& _other)
        {
            Resize(_other.m_count);
            memcpy(m_array, _other.m_array, sizeof(T) * m_count);
            return *this;
        }

        DynamicArray(DynamicArray&& _other) noexcept
        {
            m_count = _other.m_count;
            m_array = _other.m_array;

            _other.m_count = 0;
            _other.m_array = nullptr;
        }

        DynamicArray& operator=(DynamicArray&& _other) noexcept
        {
            m_count = _other.m_count;
            m_array = _other.m_array;

            _other.m_count = 0;
            _other.m_array = nullptr;

            return *this;
        }

        virtual ~DynamicArray()
        {
            Resize(0);
        }

        [[nodiscard]] u64 Size() const
        {
            return m_count;
        }

        void Resize(u64 _count)
        {
            if (m_array != nullptr)
            {
                Clear();
            }

            m_count = _count;

            if (m_count > 0)
            {
                const u64 size = sizeof(T) * m_count;
                m_buffer = new u8[size];
            }
        }

        template<class... Args>
        Ptr Init(u64 _index, Args... args)
        {
            KE_ASSERT_MSG(_index < m_count, "Beyond max index!");
            Ptr memSpace = m_array + _index;
            return new ((void*)memSpace) T(args...);
        }

        template<class... Args>
        void InitAll(Args... args)
        {
            for (Ptr valuePtr = begin(); valuePtr != end(); valuePtr++)
            {
                new ((void*)valuePtr) T(args...);
            }
        }

        void SetAll(const T& _value)
        {
            for (auto& value: *this)
            {
                value = _value;
            }
        }

        Ref operator[](u64 _index) const
        {
            KE_ASSERT_MSG(_index < m_count, "Beyond max index!");
            return m_array[_index];
        }

        Iterator begin() const
        {
            return m_array;
        }

        Iterator end() const
        {
            return m_array + m_count;
        }

        void Clear()
        {
            for (Ref instance: *this)
            {
                instance.~T();
            }
            ResetLooseMemory();
        }

        void ResetLooseMemory()
        {
            delete m_buffer;
            m_buffer = nullptr;
            m_count = 0;
        }

        Ptr Data()
        {
            return m_array;
        }

    private:
        union
        {
            u8* m_buffer = nullptr;
            Ptr m_array;
        };
        u64 m_count = 0;
    };
}
