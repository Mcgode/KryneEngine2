/**
 * @file
 * @author Max Godefroy
 * @date 18/09/2024.
 */


#include <Common/Assert.hpp>
#include <gtest/gtest.h>

using namespace KryneEngine;

TEST(Assert, SetCustomCallback)
{
    // -----------------------------------------------------------------------
    // Setup
    // -----------------------------------------------------------------------

    constexpr auto callback0 = [](const char*, u32, const char*, const char* message)
    {
        EXPECT_STREQ(message, "Callback 0");
        return true;
    };

    constexpr auto callback1 = [](const char*, u32, const char*, const char* message)
    {
        EXPECT_STREQ(message, "Callback 1");
        return true;
    };

    constexpr auto callback2 = [](const char*, u32, const char*, const char* message)
    {
        EXPECT_STREQ(message, "Callback 2");
        return false;
    };


    // -----------------------------------------------------------------------
    // Execute
    // -----------------------------------------------------------------------

    Assertion::AssertionCallback previousCallback = Assertion::SetAssertionCallback(callback0);
    EXPECT_EQ(previousCallback, nullptr); // Should be nullptr, aka default callback

    bool result = Assertion::Error("", 0, "", "Callback 0");
    EXPECT_TRUE(result);

    previousCallback = Assertion::SetAssertionCallback(callback1);
    EXPECT_EQ(previousCallback, callback0);

    result = Assertion::Error("", 0, "", "Callback 1");
    EXPECT_TRUE(result);

    previousCallback = Assertion::SetAssertionCallback(callback2);
    EXPECT_EQ(previousCallback, callback1);

    result = Assertion::Error("", 0, "", "Callback 2");
    EXPECT_FALSE(result);

    // -----------------------------------------------------------------------
    // Teardown
    // -----------------------------------------------------------------------

    Assertion::SetAssertionCallback(nullptr);
}