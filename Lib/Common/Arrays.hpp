/**
 * @file
 * @author Max Godefroy
 * @date 23/04/2022.
 */

#include <Common/KETypes.hpp>
#include <Common/Assert.hpp>

namespace KryneEngine
{
    template<class T>
    struct DynamicArray
    {
        using Ptr = T*;
        using Ref = T&;
        using Iterator = Ptr;

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
            Assert(_index < m_count, "Beyond max index!");
            void* memSpace = m_array + _index;
            return new (memSpace) T(args...);
        }

        Ref operator[](u64 _index) const
        {
            Assert(_index < m_count, "Beyond max index!");
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
            delete m_buffer;
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
