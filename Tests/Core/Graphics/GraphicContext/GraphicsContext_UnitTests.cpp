/**
* @file
* @author Max Godefroy
* @date 11/10/2024.
*/

#include <gtest/gtest.h>
#include <Utils/AssertUtils.hpp>

#include <Graphics/Common/GraphicsContext.hpp>

#include "Common.h"

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
            GraphicsContext context(appInfo, nullptr);
        }

        // -----------------------------------------------------------------------
        // Teardown
        // -----------------------------------------------------------------------

        catcher.ExpectNoMessage();
    }
}
