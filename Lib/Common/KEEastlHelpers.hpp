/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#pragma once

#include <EASTL/algorithm.h>

namespace KryneEngine::EastlHelpers
{
    template<class StdContainer, class EastlContainer, class InsertIterator>
    inline void CopyToEastlContainer(const StdContainer& _srcContainer, EastlContainer& _dstContainer, InsertIterator _insertIterator)
    {
        static_assert(std::is_same<typename EastlContainer::value_type, typename StdContainer::value_type>::value);
        _dstContainer.clear();
        _dstContainer.reserve(_srcContainer.size());
        eastl::copy(_srcContainer.begin(), _srcContainer.end(), _insertIterator);
    }

    template<class StdContainer, class EastlContainer>
    inline void CopyToEastlBackInsertingContainer(const StdContainer& _srcContainer, EastlContainer& _dstContainer)
    {
        CopyToEastlContainer(_srcContainer, _dstContainer, eastl::back_inserter(_dstContainer));
    }
}