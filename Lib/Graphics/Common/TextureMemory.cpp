/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#include <Common/Assert.hpp>
#include "TextureMemory.hpp"

namespace KryneEngine
{
    TextureMemory::TextureMemory(bool _manualDestroy)
        : m_manualDestroy(_manualDestroy)
        , m_userCount(0)
    {
    }

    TextureMemory::~TextureMemory()
    {
        Assert(m_beforeDestructCalled);
        Assert(m_userCount == 0);
    }

    void TextureMemory::_BeforeDestruct()
    {
        Assert(!m_beforeDestructCalled);

        if (!m_manualDestroy)
        {
            _Destroy();
        }
        m_beforeDestructCalled = true;
    }
}