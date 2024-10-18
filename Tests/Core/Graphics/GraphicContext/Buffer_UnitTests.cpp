/**
 * @file
 * @author Max Godefroy
 * @date 18/10/2024.
 */

#include <gtest/gtest.h>
#include <Utils/AssertUtils.hpp>

#include <Graphics/Common/Buffer.hpp>
#include <Graphics/Common/GraphicsContext.hpp>

#include "Common.h"

namespace KryneEngine::Tests::Graphics
{
    const BufferCreateDesc defaultBufferCreateDesc {
        .m_desc = {
            .m_size = 16,
            .m_debugName = "Unit Test Buffer 0",
        },
        .m_usage = MemoryUsage::GpuOnly_UsageType | MemoryUsage::TransferDstBuffer
    };

    TEST(Buffer, CreateBuffer)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        const GraphicsCommon::ApplicationInfo appInfo = DefaultAppInfo();
        auto graphicsContext = eastl::make_unique<GraphicsContext>(appInfo, nullptr);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        u32 errorCount = 0;

        {
            const BufferHandle buffer = graphicsContext->CreateBuffer(defaultBufferCreateDesc);
            catcher.ExpectMessageCount(errorCount);
            EXPECT_NE(buffer, GenPool::kInvalidHandle);
            EXPECT_EQ(buffer.m_handle.m_index, 0);
            EXPECT_EQ(buffer.m_handle.m_generation, 0);
        }

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        graphicsContext.reset();

#if defined(KE_GRAPHICS_API_VK)
        // One error from validation layers, non-destroyed buffer
        if (catcher.GetCaughtMessages().size() < errorCount)
        {
            const eastl::string& message = catcher.GetCaughtMessages()[errorCount].m_message;
            EXPECT_TRUE(
                message.find("Validation Error: [ VUID-vkDestroyDevice-device-05137 ]") != eastl::string::npos
                && message.find("name = Unit Test Buffer 0") != eastl::string::npos);
        }
        errorCount += 1;

        // One error from VMA leaks check
        if (catcher.GetCaughtMessages().size() < errorCount)
        {
            const eastl::string& message = catcher.GetCaughtMessages()[errorCount].m_message;
            EXPECT_NE(
                message.find("\"Some allocations were not freed before destruction of this memory block!\""),
                eastl::string::npos);
        }
        errorCount += 1;
#elif defined(KE_GRAPHICS_API_DX12)
        // No leak warning in DirectX 12 :(
#endif

        catcher.ExpectMessageCount(errorCount);
    }

    TEST(Buffer, DestroyBuffer)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        const GraphicsCommon::ApplicationInfo appInfo = DefaultAppInfo();
        auto graphicsContext = eastl::make_unique<GraphicsContext>(appInfo, nullptr);
        const BufferHandle buffer = graphicsContext->CreateBuffer(defaultBufferCreateDesc);

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        EXPECT_FALSE(graphicsContext->DestroyBuffer({ GenPool::kInvalidHandle }));
        EXPECT_TRUE(graphicsContext->DestroyBuffer(buffer));
        EXPECT_FALSE(graphicsContext->DestroyBuffer(buffer));

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        graphicsContext.reset();
        catcher.ExpectNoMessage();
    }
}
