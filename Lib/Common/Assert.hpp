/**
 * @file
 * @author Max Godefroy
 * @date 02/08/2021.
 */

#pragma once

namespace KryneEngine::Assertion
{
	bool Error(const char* _function, u32 _line, const char* _file, const char* _formatMessage, ...);
}

#define KE_ASSERT_MSG(condition, message, args...) do \
	{ \
		if (!(condition)) [[unlikely]] \
		{\
			if (KryneEngine::Assertion::Error(__builtin_FUNCTION(), __builtin_LINE(), __builtin_FILE(), message, args)) \
			{ \
				__debugbreak(); \
			} \
		} \
	} \
	while(0)
#define KE_ASSERT(condition) KE_ASSERT_MSG(condition, #condition)

#define KE_VERIFY_MSG(condition, message, args...) ((condition) ? true: \
	KryneEngine::Assertion::Error(__builtin_FUNCTION(), __builtin_LINE(), __builtin_FILE(), message, args) ? __debugbreak(), false: false)
#define KE_VERIFY(condition) KE_VERIFY_MSG(condition, #condition)

#define KE_ERROR(message, args...) do { if (KryneEngine::Assertion::Error(__builtin_FUNCTION(), __builtin_LINE(), __builtin_FILE(), message, args)) __debugbreak(); } while (0)

#define IF_NOT_VERIFY(cond) if (!KE_VERIFY(cond)) [[unlikely]]
#define IF_NOT_VERIFY_MSG(cond, message, args...) if (!KE_VERIFY_MSG(cond, message, args)) [[unlikely]]
#define VERIFY_OR_RETURN(cond, returnValue) IF_NOT_VERIFY(cond) return returnValue
#define VERIFY_OR_RETURN_VOID(cond)  VERIFY_OR_RETURN(cond, )