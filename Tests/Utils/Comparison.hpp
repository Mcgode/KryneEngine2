/**
* @file
* @author Max Godefroy
* @date 13/10/2024.
 */

#pragma once

#include <EASTL/string.h>

#define EXPECT_BINARY_EQ(_a, _b) EXPECT_TRUE(BinaryCompare(_a, _b)) << GetBinaryDifferenceErrorString(_a, _b).c_str()

namespace KryneEngine::Tests
{
    template <class T>
    bool BinaryCompare(const T* _a, const T* _b)
    {
        return memcmp(_a, _b, sizeof(T)) == 0;
    }

    template <class T>
    bool BinaryCompare(const T& _a, const T& _b)
    {
        return BinaryCompare<T>(&_a, &_b);
    }

    template <class T>
    eastl::string GetBinaryDifferenceErrorString(const T* _a, const T* _b)
    {
        constexpr size_t maxFullPrintSize = 16;

        eastl::string result = "";

        const auto appendByte = [&result](const T* _obj, size_t _index)
        {
            result.append_sprintf(" %02X", reinterpret_cast<const u8*>(_obj)[_index]);
        };

        if constexpr (sizeof(T) <= maxFullPrintSize)
        {
            result += "Expected:";
            for (size_t  i = 0; i < sizeof(T); i++) { appendByte(_a, i); }
            result += "\n";

            result += "Got:     ";
            for (size_t  i = 0; i < sizeof(T); i++) { appendByte(_b, i); }
        }
        else
        {
            const auto* a = reinterpret_cast<const u8*>(_a);
            const auto* b = reinterpret_cast<const u8*>(_b);

            size_t i = 0;
            for (; i < sizeof(T) && a[i] == b[i]; i++);

            if (i != sizeof(T))
            {
                result.append_sprintf("Difference starting at index %llu\n", i);

                constexpr size_t offset = 4;

                size_t start = 0;
                bool trimStart = false;
                if (i >= offset)
                {
                    start = i - offset;
                    trimStart = true;
                }

                size_t end = sizeof(T);
                bool trimEnd = false;
                if (start + maxFullPrintSize < sizeof(T))
                {
                    end = start + maxFullPrintSize;
                    trimEnd = true;
                }

                result += "Expected:";
                if (trimStart) result += " ...";
                for (size_t j = start; j < end; j++) appendByte(_a, j);
                if (trimEnd) result += " ...";

                result += "\n";

                result += "Got:     ";
                if (trimStart) result += " ...";
                for (size_t j = start; j < end; j++) appendByte(_b, j);
                if (trimEnd) result += " ...";
            }
        }

        return result;
    }

    template <class T>
    eastl::string GetBinaryDifferenceErrorString(const T& _a, const T& _b)
    {
        return GetBinaryDifferenceErrorString<T>(&_a, &_b);
    }
}
