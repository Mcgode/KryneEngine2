/**
 * @file
 * @author Max Godefroy
 * @date 18/09/2024.
 */

#pragma once

#include <Common/Assert.hpp>

namespace KryneEngine::Tests
{
    class ScopedAssertCatcher
    {
        friend class AssertUtils_ProperScoping_Test;

    public:
        struct Messages
        {
            eastl::string m_message;
            eastl::string m_functionName;
            eastl::string m_fileName;
            u32 m_lineIndex;
        };

        ScopedAssertCatcher();

        ~ScopedAssertCatcher();

        [[nodiscard]] eastl::span<const Messages> GetCaughtMessages() const { return m_caughtMessages; }
        [[nodiscard]] const Messages& GetLastCaughtMessages() const { return m_caughtMessages.back(); }

    private:
        static bool Callback(const char* _function, uint32_t _line, const char* _file, const char* _message);

        KryneEngine::Assertion::AssertionCallback m_previousCallback = nullptr;
        ScopedAssertCatcher* m_previousCatcher = nullptr;

        static ScopedAssertCatcher* s_currentCatcher;
        eastl::vector<Messages> m_caughtMessages;
    };
}