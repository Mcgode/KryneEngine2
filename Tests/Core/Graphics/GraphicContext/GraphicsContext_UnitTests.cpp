/**
* @file
* @author Max Godefroy
* @date 11/10/2024.
*/

#include "KryneEngine/Core/Graphics/GraphicsContext.hpp"
#include <gtest/gtest.h>

#include "Common.h"
#include "Utils/AssertUtils.hpp"
#include "Utils/Comparison.hpp"

namespace KryneEngine::Tests::Graphics
{
    TEST(GraphicsContext, Creation)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        const GraphicsCommon::ApplicationInfo appInfo = DefaultAppInfo();

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        {
            auto* context = GraphicsContext::Create(appInfo, nullptr, AllocatorInstance());
            GraphicsContext::Destroy(context);
        }

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        catcher.ExpectNoMessage();
    }

    TEST(GraphicsContext, GetFrameContextCount)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        GraphicsCommon::ApplicationInfo appInfo = DefaultAppInfo();

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        // When no swap chain is provided, frame context count should always be 2

        {
            appInfo.m_displayOptions.m_tripleBuffering = GraphicsCommon::SoftEnable::Disabled;
            GraphicsContext* graphicsContext = GraphicsContext::Create(appInfo, nullptr, AllocatorInstance());
            EXPECT_EQ(graphicsContext->GetFrameContextCount(), 2);
            GraphicsContext::Destroy(graphicsContext);
        }

        {
            appInfo.m_displayOptions.m_tripleBuffering = GraphicsCommon::SoftEnable::TryEnable;
            GraphicsContext* graphicsContext = GraphicsContext::Create(appInfo, nullptr, AllocatorInstance());
            EXPECT_EQ(graphicsContext->GetFrameContextCount(), 2);
            GraphicsContext::Destroy(graphicsContext);
        }

        {
            appInfo.m_displayOptions.m_tripleBuffering = GraphicsCommon::SoftEnable::ForceEnabled;
            GraphicsContext* graphicsContext = GraphicsContext::Create(appInfo, nullptr, AllocatorInstance());
            EXPECT_EQ(graphicsContext->GetFrameContextCount(), 2);
            GraphicsContext::Destroy(graphicsContext);
        }

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        catcher.ExpectNoMessage();
    }

    TEST(GraphicsContext, GetApplicationInfo)
    {
        // -----------------------------------------------------------------------
        // Setup
        // -----------------------------------------------------------------------

        ScopedAssertCatcher catcher;
        const GraphicsCommon::ApplicationInfo appInfo = DefaultAppInfo();
        GraphicsContext* graphicsContext = GraphicsContext::Create(appInfo, nullptr, AllocatorInstance());

        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        const GraphicsCommon::ApplicationInfo gAppInfo = graphicsContext->GetApplicationInfo();

        EXPECT_EQ(appInfo.m_applicationName, gAppInfo.m_applicationName);
        EXPECT_BINARY_EQ(appInfo.m_applicationVersion, gAppInfo.m_applicationVersion);

        EXPECT_BINARY_EQ(appInfo.m_engineVersion, gAppInfo.m_engineVersion);
        EXPECT_EQ(appInfo.m_api, appInfo.m_api);

        EXPECT_BINARY_EQ(appInfo.m_features, gAppInfo.m_features);
        EXPECT_BINARY_EQ(appInfo.m_displayOptions, gAppInfo.m_displayOptions);

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        GraphicsContext::Destroy(graphicsContext);

        catcher.ExpectNoMessage();
    }
}
