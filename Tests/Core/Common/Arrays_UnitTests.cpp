/**
 * @file
 * @author Max Godefroy
 * @date 18/09/2024.
 */

#include <gtest/gtest.h>

#include <Common/Arrays.hpp>
#include <Utils/AssertUtils.hpp>

namespace KryneEngine::Tests
{
    TEST(DynamicArray, Size)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        using T = u32;
        DynamicArray<T> dynamicArray;

        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        EXPECT_TRUE(dynamicArray.Empty());
        EXPECT_EQ(dynamicArray.Size(), 0);
        EXPECT_EQ(dynamicArray.Data(), nullptr);

        dynamicArray.Resize(1);

        EXPECT_FALSE(dynamicArray.Empty());
        EXPECT_EQ(dynamicArray.Size(), 1);
        EXPECT_NE(dynamicArray.Data(), nullptr);

        T* dataPtr = dynamicArray.Data();
        dynamicArray.Resize(100);

        EXPECT_EQ(dynamicArray.Size(), 100);
        EXPECT_NE(dynamicArray.Data(), nullptr);
        EXPECT_NE(dynamicArray.Data(), dataPtr);

        dynamicArray.Clear();

        EXPECT_TRUE(dynamicArray.Empty());
        EXPECT_EQ(dynamicArray.Size(), 0);
        EXPECT_EQ(dynamicArray.Data(), nullptr);

        ASSERT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(DynamicArray, Access)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        using T = u32;
        DynamicArray<T> dynamicArray;
        dynamicArray.Resize(10);

        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        T* ptr = dynamicArray.Data();

        for (u32 i = 0; i < dynamicArray.Size(); i++)
        {
            EXPECT_EQ(ptr[i], dynamicArray[i]);
        }

        EXPECT_EQ(ptr, &dynamicArray[0]);

        dynamicArray[3] = 12;

        EXPECT_EQ(ptr[3], 12);

        ASSERT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(DynamicArray, Iterator)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        using T = u32;
        DynamicArray<T> dynamicArray;
        dynamicArray.Resize(10);

        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        for (u32 i = 0; i < dynamicArray.Size(); i++)
        {
            dynamicArray[i] = i;
        }

        u32 i = 0;
        auto it = dynamicArray.begin();
        EXPECT_EQ(it, dynamicArray.Data());
        for (; i < dynamicArray.Size() && it != dynamicArray.end(); i++, it++)
        {
            EXPECT_EQ(dynamicArray[i], *it);
        }
        EXPECT_EQ(it, dynamicArray.end());

        ASSERT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(DynamicArray, ComplexCreate)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        using T = u32;

        constexpr T refArray[] = { 4, 8, 15, 16, 23, 42 };
        constexpr size_t refSize = eastl::size(refArray);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        DynamicArray<T> sizedDynamicArray = DynamicArray<T>(refSize);
        EXPECT_EQ(sizedDynamicArray.Size(), refSize);
        // With this constructor, the content is uninitialized, so only compare sizes.

        DynamicArray<T> uniqueValueDynamicArray = DynamicArray<T>(refSize, refArray[0]);
        EXPECT_EQ(uniqueValueDynamicArray.Size(), refSize);
        for (T& value: uniqueValueDynamicArray)
        {
            EXPECT_EQ(value, refArray[0]);
        }

        DynamicArray<T> initializerListDynamicArray = {
            refArray[0],
            refArray[1],
            refArray[2],
            refArray[3],
            refArray[4],
            refArray[5],
        };
        EXPECT_EQ(initializerListDynamicArray.Size(), refSize);
        for (u32 i = 0; i < refSize; i++)
        {
            EXPECT_EQ(refArray[i], initializerListDynamicArray[i]);
        }

        ASSERT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(DynamicArray, Clear_Vs_ResetLooseMemory)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        static std::atomic<u32> count = 0;
        struct RefCounted
        {
            RefCounted() { count++; }
            ~RefCounted() { count--; }
        };
        ASSERT_EQ(count, 0) << "Did not reset test properly";

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        DynamicArray<RefCounted> array(10);

        EXPECT_EQ(count, 0);

        array.InitAll();

        EXPECT_EQ(count, 10);

        array.Clear();

        EXPECT_EQ(count, 0);

        array.Resize(10);
        array.InitAll();

        EXPECT_EQ(count, 10);

        array.ResetLooseMemory();

        EXPECT_EQ(count, 10);

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        count = 0; // In case the test is re-run
    }
}
