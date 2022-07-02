/**
 * @file
 * @author Max Godefroy
 * @date 02/07/2022.
 */

#pragma once

namespace KryneEngine
{
    struct Context
    {
        void *rip, *rsp;
        void *rbx, *rbp, *r12, *r13, *r14, *r15;
    };

    void GetContext(Context* _current);
    void SetContext(Context* _new);
    void SwapContext(Context* _current, Context* _new);
}