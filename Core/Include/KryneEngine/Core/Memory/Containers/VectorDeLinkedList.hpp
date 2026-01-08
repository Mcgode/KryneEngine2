/**
 * @file
 * @author Max Godefroy
 * @date 08/01/2026.
 */

#pragma once

#include <EASTL/vector.h>

#include "KryneEngine/Core/Common/Assert.hpp"
#include "KryneEngine/Core/Common/Types.hpp"
#include "KryneEngine/Core/Memory/Allocators/Allocator.hpp"

namespace KryneEngine
{

    /**
     * @brief A templated class representing a double-ended linked list that uses an internal vector for storage.
     * The class supports both intrusive and non-intrusive types for nodes.
     *
     * @tparam T The type of elements stored in the list. For intrusive nodes, it must have a `m_next` u32 member.
     * @tparam Intrusive A boolean indicating whether the list operates in intrusive mode. Defaults to true if type `T`
     * has a valid `m_next` member.
     *
     * @details
     * The user is in charge of linking allocated nodes together. The class design has been kept minimal to allow for
     * flexible usage and customization.
     * The indices are stable over time. When freeing an index, it is added to the freelist for reuse.
     */
    template <class T, bool Intrusive = std::is_same_v<u32, decltype(T::m_next)> && std::is_same_v<u32, decltype(T::m_previous)>>
    class VectorDeLinkedList
    {
    public:
        explicit VectorDeLinkedList(AllocatorInstance _allocator): m_vector(_allocator) {}

        VectorDeLinkedList(const VectorDeLinkedList&) = default;
        VectorDeLinkedList(VectorDeLinkedList&&) = default;
        VectorDeLinkedList& operator=(const VectorDeLinkedList&) = default;
        VectorDeLinkedList& operator=(VectorDeLinkedList&&) = default;

        /// Special constant value used to indicate the beginning or the end of a linked list.
        static constexpr u32 kListLimitId = ~0u;

        T& operator[](u32 _index)
        {
            if constexpr (Intrusive)
                return m_vector[_index];
            else
                return m_vector[_index].m_value;
        }

        const T& operator[](u32 _index) const
        {
            if constexpr (Intrusive)
                return m_vector[_index];
            else
                return m_vector[_index].m_value;
        }

        u32 AllocateNode()
        {
            if (m_firstFree == kListLimitId)
            {
                m_vector.emplace_back();
                m_vector.back().m_next = kListLimitId;
                m_vector.back().m_previous = kListLimitId;
                return m_vector.size() - 1;
            }
            else
            {
                const u32 result = m_firstFree;
                m_firstFree = m_vector[result].m_next;
                KE_ASSERT(m_vector[result].m_previous == kListLimitId);
                m_vector[result].m_previous = kListLimitId;
                m_vector[result].m_next = kListLimitId;
                return result;
            }
        }

        T& Allocate()
        {
            return operator[](AllocateNode());
        }

        [[nodiscard]] u32 GetNext(u32 _index) const
        {
            return m_vector[_index].m_next;
        }

        [[nodiscard]] u32 GetPrevious(u32 _index) const
        {
            return m_vector[_index].m_previous;
        }

        void SetNext(u32 _index, u32 _nextIndex) const
        {
            m_vector[_index].m_next = _nextIndex;
            m_vector[_nextIndex].m_previous = _index;
        }

        void FreeNode(u32 _index)
        {
            Node& node = m_vector[_index];
            if (node.m_previous != kListLimitId)
                m_vector[node.m_previous].m_next = node.m_next;
            node.m_previous = kListLimitId;
            node.m_next = m_firstFree;
            m_firstFree = _index;
        }

    private:
        struct NonIntrusiveNode
        {
            T m_value;
            u32 m_next;
            u32 m_previous;
        };

        using Node = std::conditional_t<Intrusive, T, NonIntrusiveNode>;

        eastl::vector<Node> m_vector;
        u32 m_firstFree = kListLimitId;
    };
}