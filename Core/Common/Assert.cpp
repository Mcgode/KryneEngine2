#include "Assert.hpp"

#include <EASTL/vector_set.h>
#include <Threads/LightweightMutex.hpp>

#include "StringHelpers.hpp"

#if defined(_WIN32)
#	include <Platform/Windows.h>
#endif

namespace KryneEngine::Assertion
{
	static eastl::vector_set<u64> g_ignoredIds;
	static LightweightMutex g_mutex;

	bool Error(const char* _function, u32 _line, const char* _file, const char* _formatMessage, ...)
	{
#if defined(_WIN32)
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

		eastl::string message;
		message.sprintf("Assertion failed in %s (at %s):\n\n\t", _function, location.c_str());

		va_list arguments;
		va_start(arguments, _formatMessage);
		message.append_sprintf_va_list(_formatMessage, arguments);
		va_end(arguments);

		const auto messageBoxId = MessageBoxA(
			nullptr,
			message.c_str(),
			"Assertion failed!",
			MB_ICONSTOP | MB_YESNOCANCEL | MB_DEFBUTTON1
		);

		switch (messageBoxId)
		{
		case IDCANCEL:
			{
				const auto lock = g_mutex.AutoLock();
				g_ignoredIds.emplace(id);
			}
			[[fallthrough]];
		case IDNO:
			return false;
		}
#endif
		return true;
	}
}
