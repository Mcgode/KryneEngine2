#include "Assert.hpp"

#include <EASTL/vector_set.h>
#include <Threads/LightweightMutex.hpp>

#include "StringHelpers.hpp"

#if defined(_WIN32)
#	include <Platform/Windows.h>
#endif

namespace KryneEngine::Assertion
{
    static AssertionCallback g_assertionCallback = nullptr;
    static eastl::vector_set<u64> g_ignoredIds;
	static LightweightMutex g_mutex;

    CallbackResponse DefaultAssertCallback(const char* _function, u32 _line, const char* _file, const char* _message)
    {
#if defined(_WIN32)
        eastl::string message;
        message.sprintf("Assertion failed in %s (at %s:%d):\n\n\t%s", _function, _file, _line, _message);

        const auto messageBoxId = MessageBoxA(
            nullptr,
            message.c_str(),
            "Assertion failed!",
            MB_ICONSTOP | MB_YESNOCANCEL | MB_DEFBUTTON1
        );

        switch (messageBoxId)
        {
        case IDCANCEL:
            return CallbackResponse::Ignore;
        case IDNO:
            return CallbackResponse::Continue;
        default:
            return CallbackResponse::Break;
        }
#endif
    }

	bool Error(const char* _function, u32 _line, const char* _file, const char* _formatMessage, ...)
	{
        eastl::string message = "";
        va_list arguments;
        va_start(arguments, _formatMessage);
        message.sprintf_va_list(_formatMessage, arguments);
        va_end(arguments);

        eastl::string location;
        location.sprintf("%s:%d", _file, _line);

        const u64 id = StringHash::Hash64(location);
        {
            const auto lock = g_mutex.AutoLock();
            if (g_ignoredIds.find(id) != g_ignoredIds.end())
            {
                return false;
            }
        }

        const AssertionCallback callback = g_assertionCallback != nullptr ? g_assertionCallback : DefaultAssertCallback;
        const CallbackResponse response = callback(_function, _line, _file, message.c_str());

        switch (response)
        {
        case CallbackResponse::Ignore:
            {
                const auto lock = g_mutex.AutoLock();
                g_ignoredIds.emplace(id);
            }
            [[fallthrough]];
        case CallbackResponse::Continue:
            return false;
        default:
            return true;
        }
	}

    AssertionCallback SetAssertionCallback(AssertionCallback _userCallback)
    {
        AssertionCallback previous = g_assertionCallback;
        g_assertionCallback = _userCallback;
        return previous;
    }
}
