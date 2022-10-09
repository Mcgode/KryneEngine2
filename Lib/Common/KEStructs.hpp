/**
 * @file
 * @author Max Godefroy
 * @date 26/03/2022.
 */

#pragma once

#include <Common/Assert.hpp>
#include <Common/KETypes.hpp>
#include <atomic>

namespace KryneEngine
{
    template <typename T, class Destructor>
    struct SharedObject
    {
    public:
        struct Ref
        {
            friend SharedObject;

        public:
            SharedObject* m_sharedObject = nullptr;

            Ref() = default;

            Ref(const Ref& _other)
            {
                m_sharedObject = _other.m_sharedObject;

                if (m_sharedObject)
                {
                    m_sharedObject->m_referencesCount++;
                }
            }

            Ref& operator=(const Ref& _other)
            {
                if (&_other == this) [[unlikely]]
                {
                    return *this;
                }

                _Unref();

                m_sharedObject = _other.m_sharedObject;

                if (m_sharedObject)
                {
                    m_sharedObject->m_referencesCount++;
                }
            }

            Ref(Ref&& _other) noexcept
                : m_sharedObject(_other.m_sharedObject)
            {
                _other.m_sharedObject = nullptr;
            }

            Ref& operator=(Ref&& _other) noexcept
            {
                _Unref();
                m_sharedObject = _other.m_sharedObject;
                _other.m_sharedObject = nullptr;
                return *this;
            }

            inline bool operator!() const { return m_sharedObject == nullptr; }
            inline bool operator==(std::nullptr_t) const { return m_sharedObject == nullptr; }
            inline bool operator!=(std::nullptr_t) const { return m_sharedObject != nullptr; }

            virtual ~Ref()
            {
                _Unref();
            }

            T* operator->() const
            {
                return &m_sharedObject->m_object;
            }

            T& operator*() const
            {
                return m_sharedObject->m_object;
            }

            void Reset()
            {
                _Unref();
                m_sharedObject = nullptr;
            }

        private:
            explicit Ref(SharedObject* _sharedObject)
                    : m_sharedObject(_sharedObject)
            {
                _sharedObject->m_referencesCount++;
            }

            void _Unref()
            {
                if (m_sharedObject != nullptr)
                {
                    const s32 refCount = --m_sharedObject->m_referencesCount;
                    Assert(refCount >= 0, "Ref and unref mismatch");
                }
            }
        };

    public:
        T m_object;

        explicit SharedObject(T&& _instance, Destructor _destructor = Destructor())
                : m_object(_instance)
                , m_destructor(_destructor)
        {}

        virtual ~SharedObject()
        {
            Destroy();
        }

        T* operator->()
        {
            return &m_object;
        }

        T& operator*()
        {
            return m_object;
        }

        void Destroy()
        {
            bool expected = false;
            if (!m_destroyed.compare_exchange_strong(expected, true))
            {
                return;
            }

            Assert(m_referencesCount <= 0, "Deleting shared object while there are still dangling references");
            m_destructor(m_object);
        }

        Ref MakeRef()
        {
            return Ref(this);
        }

    private:
        std::atomic<bool> m_destroyed = false;
        std::atomic<s32> m_referencesCount = 0;
        Destructor m_destructor;
    };
}