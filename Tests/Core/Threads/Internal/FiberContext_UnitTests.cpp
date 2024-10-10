/**
* @file
* @author Max Godefroy
* @date 10/10/2024.
*/

#include <gtest/gtest.h>

#include <Threads/Internal/FiberContext.hpp>
#include <Utils/AssertUtils.hpp>

namespace KryneEngine::Tests
{
    TEST(FiberContextAllocator, Init)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_NO_THROW(FiberContextAllocator allocator);

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }

    TEST(FiberContextAllocator, GetContext)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        FiberContextAllocator allocator;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        u16 i = 0;
        for (; i < FiberContextAllocator::kSmallStackCount + FiberContextAllocator::kBigStackCount; i++)
        {
            FiberContext* ctx = allocator.GetContext(i);
            EXPECT_NE(ctx, nullptr);
            if (ctx != nullptr)
            {
#if CONTEXT_SWITCH_WINDOWS_FIBERS
                EXPECT_NE(ctx->m_winFiber, nullptr);
#else
#   error No test case yet
#endif
            }
        }

        EXPECT_TRUE(catcher.GetCaughtMessages().empty());

        EXPECT_EQ(allocator.GetContext(i), nullptr);
        EXPECT_EQ(catcher.GetCaughtMessages().size(), 1);
    }

    TEST(FiberContextAllocator, Allocate)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        FiberContextAllocator allocator;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        u32 expectedErrorCount = 0;

        const auto testStackType = [&](const u16 count, const bool bigStack)
        {
            constexpr u16 initial = ~0;
            u16 id = initial;

            for (u16 i = 0; i < count; i++)
            {
                const u16 prevId = id;
                EXPECT_TRUE(allocator.Allocate(bigStack, id));

                if (prevId != initial)
                {
                    EXPECT_LE(prevId, id);
                }
            }

            EXPECT_EQ(catcher.GetCaughtMessages().size(), expectedErrorCount);

            const u16 prevId = id;
            EXPECT_FALSE(allocator.Allocate(bigStack, id));
            expectedErrorCount++;
            EXPECT_EQ(prevId, id);
            EXPECT_EQ(catcher.GetCaughtMessages().size(), expectedErrorCount);
            EXPECT_EQ(catcher.GetLastCaughtMessages().m_message, "Out of Fiber stacks!");
        };

        testStackType(FiberContextAllocator::kSmallStackCount, false);
        testStackType(FiberContextAllocator::kBigStackCount, true);
    }

    TEST(FiberContextAllocator, Free)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        FiberContextAllocator allocator;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        const auto testStackType = [&](const u16 count, const bool bigStack)
        {
            u16 id = 0;
            u16 first = 0;

            allocator.Allocate(bigStack, first);
            for (u16 i = 1; i < count; i++)
            {
                EXPECT_TRUE(allocator.Allocate(bigStack, id));
            }

            allocator.Free(first);
            EXPECT_TRUE(allocator.Allocate(bigStack, id));
            EXPECT_EQ(id, first);

            // Make sure priority queue works appropriately
            allocator.Free(first + 1);
            allocator.Free(first);
            EXPECT_TRUE(allocator.Allocate(bigStack, id));
            EXPECT_EQ(id, first);

            EXPECT_TRUE(catcher.GetCaughtMessages().empty());
        };

        testStackType(FiberContextAllocator::kSmallStackCount, false);
        testStackType(FiberContextAllocator::kBigStackCount, true);

        allocator.Free(FiberContextAllocator::kBigStackCount + FiberContextAllocator::kSmallStackCount);
        EXPECT_EQ(catcher.GetCaughtMessages().size(), 1);
    }

    TEST(FiberContext, SwapContext)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        struct Contexts
        {
            FiberContext starting;
            FiberContext target;
        } contexts;

        contexts.starting.m_name = "Starting";
        contexts.target.m_name = "Target";

        constexpr auto targetFunction = [](void* _contexts)
        {
            auto* contexts = static_cast<Contexts*>(_contexts);

            contexts->target.m_name = "Targeted";

            contexts->target.SwapContext(&contexts->starting);
        };

        std::thread startThread(
            [&](){
#if CONTEXT_SWITCH_WINDOWS_FIBERS
                contexts.starting.m_winFiber = ConvertThreadToFiber(nullptr);
                contexts.target.m_winFiber = CreateFiber(1 << 16, targetFunction, &contexts);

                contexts.starting.SwapContext(&contexts.target);
                ConvertFiberToThread();
#else
#   error No test yet
#endif
            });
        startThread.join();

        EXPECT_EQ(contexts.target.m_name, "Targeted");

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

#if CONTEXT_SWITCH_WINDOWS_FIBERS
        DeleteFiber(contexts.target.m_winFiber);
#else
#   error No test yet
#endif
        EXPECT_TRUE(catcher.GetCaughtMessages().empty());
    }
}
