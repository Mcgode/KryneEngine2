/**
 * @file
 * @author Max Godefroy
 * @date 18/09/2024.
 */

#include "AssertUtils.hpp"

#include <gtest/gtest.h>

namespace KryneEngine::Tests
{
    ScopedAssertCatcher* ScopedAssertCatcher::s_currentCatcher = nullptr;

    ScopedAssertCatcher::ScopedAssertCatcher()
    {
        m_previousCallback = KryneEngine::Assertion::SetAssertionCallback(Callback);
        if (s_currentCatcher != nullptr)
        {
            m_previousCatcher = s_currentCatcher;
        }
        s_currentCatcher = this;
    }

    ScopedAssertCatcher::~ScopedAssertCatcher()
    {
        KryneEngine::Assertion::AssertionCallback current = KryneEngine::Assertion::SetAssertionCallback(m_previousCallback);
        EXPECT_EQ(current, Callback);

        s_currentCatcher = m_previousCatcher;
    }

    bool ScopedAssertCatcher::Callback(const char* _function, uint32_t _line, const char* _file, const char* _message)
    {
        EXPECT_NE(s_currentCatcher, nullptr);
        s_currentCatcher->m_caughtMessages.push_back({
            .m_message = _message,
            .m_functionName = _function,
            .m_fileName = _file,
            .m_lineIndex = _line,
        });
        return false; // Don't trigger debug break
    }
}
