/**
 * @file
 * @author Max Godefroy
 * @date 05/03/2025.
 */

#pragma once

#include <EASTL/span.h>

#include "KryneEngine/Core/Common/Types.hpp"

namespace KryneEngine::Hashing
{
    u64 Hash64(const std::byte* _data, size_t _size);
    u64 Hash64Append(const std::byte* _data, size_t _size, u64 _accumulatedHash);

    template<typename T>
    u64 Hash64(const T* _value)
    {
        return Hash64(reinterpret_cast<const std::byte*>(_value), sizeof(T));
    }

    template<typename T>
    u64 Hash64Append(const T* _value, u64 _accumulatedHash)
    {
        return Hash64Append(reinterpret_cast<const std::byte*>(_value), sizeof(T), _accumulatedHash);
    }

    template<typename T>
    u64 Hash64(const T& _value)
    {
        return Hash64(&_value);
    }

    template<typename T>
    u64 Hash64Append(const T& _value, u64 _accumulatedHash)
    {
        return Hash64Append(&_value, _accumulatedHash);
    }

    template<typename T>
    u64 Hash64(const T* _data, size_t _count)
    {
        return Hash64(reinterpret_cast<const std::byte*>(_data), _count * sizeof(T));
    }

    template<typename T>
    u64 Hash64Append(const T* _data, size_t _count, u64 _accumulatedHash)
    {
        return Hash64Append(reinterpret_cast<const std::byte*>(_data), _count * sizeof(T), _accumulatedHash);
    }

    template<typename T>
    u64 Hash64(eastl::span<T> _span)
    {
        return Hash64(reinterpret_cast<const std::byte*>(_span.data()), _span.size() * sizeof(T));
    }

    template<typename T>
    u64 Hash64Append(eastl::span<T> _span, u64 _accumulatedHash)
    {
        return Hash64Append(reinterpret_cast<const std::byte*>(_span.data()), _span.size() * sizeof(T), _accumulatedHash);
    }
}