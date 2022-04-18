/**
 * @file
 * @author Max Godefroy
 * @date 02/08/2021.
 */

#pragma once

#include <stdexcept>
#include <EASTL/string_view.h>


namespace KryneEngine
{
    inline void Error(const eastl::string_view& _msg)
    {
        throw std::runtime_error(_msg.begin());
    }

    inline void Assert(bool _condition, const eastl::string_view& _message)
    {
        if (!_condition)
        {
            Error(_message);
        }
    }

    inline void Assert(bool _condition)
    {
        Assert(_condition, "Condition not met");
    }

    inline bool Verify(bool _condition)
    {
        Assert(_condition);
        return _condition;
    }

    inline bool Verify(bool _condition, const eastl::string_view& _message)
    {
        Assert(_condition, _message);
        return _condition;
    }
}