/**
 * @file
 * @author Max Godefroy
 * @date 02/11/2024.
 */

#include "EnumConverters.hpp"

namespace KryneEngine::MetalConverters
{
    MTL::LoadAction GetMetalLoadOperation(RenderPassDesc::Attachment::LoadOperation _op)
    {
        switch (_op)
        {
        case RenderPassDesc::Attachment::LoadOperation::Load:
            return MTL::LoadActionLoad;
        case RenderPassDesc::Attachment::LoadOperation::Clear:
            return MTL::LoadActionClear;
        case RenderPassDesc::Attachment::LoadOperation::DontCare:
            return MTL::LoadActionDontCare;
        }
    }

    MTL::StoreAction GetMetalStoreOperation(RenderPassDesc::Attachment::StoreOperation _op)
    {
        switch (_op)
        {
        case RenderPassDesc::Attachment::StoreOperation::Store:
            return MTL::StoreActionStore;
        case RenderPassDesc::Attachment::StoreOperation::Resolve:
            return MTL::StoreActionStoreAndMultisampleResolve;
        case RenderPassDesc::Attachment::StoreOperation::DontCare:
            return MTL::StoreActionDontCare;
        }
    }
}