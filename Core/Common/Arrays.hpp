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

        explicit DynamicArray(size_t _count)
        {
            Resize(_count);
        }

        DynamicArray(size_t _count, const T& _value)
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
            for (u64 i = 0; i < m_count; i++) { m_array[i] = _other.m_array[i]; }
        }

        DynamicArray& operator=(const DynamicArray& _other)
        {
            Resize(_other.m_count);
            for (u64 i = 0; i < m_count; i++) { m_array[i] = _other.m_array[i]; }
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

        [[nodiscard]] size_t Size() const
        {
            return m_count;
        }

        [[nodiscard]] bool Empty() const
        {
            return m_count == 0;
        }

        void Resize(size_t _count)
        {
            if (m_array != nullptr)
            {
                Clear();
            }

            m_count = _count;

            if (m_count > 0)
            {
                const size_t size = sizeof(T) * m_count;
                m_buffer = new u8[size];
            }
        }

        template<class... Args>
        Ptr Init(u64 _index, Args... args)
        {
            IF_NOT_VERIFY_MSG(_index < m_count, "Beyond max index!")
                return nullptr;
            Ptr memSpace = m_array + _index;
            return new (memSpace) T(args...);
        }

        template<class... Args>
        void InitAll(Args... args)
        {
            for (Ptr valuePtr = begin(); valuePtr != end(); valuePtr++)
            {
                new (valuePtr) T(args...);
            }
        }

        void SetAll(const T& _value)
        {
            for (auto& value: *this)
            {
                value = _value;
            }
        }

        Ref operator[](size_t _index) const
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
        size_t m_count = 0;
    };
}
