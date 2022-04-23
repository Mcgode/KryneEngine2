/**
 * @file
 * @author Max Godefroy
 * @date 03/04/2022.
 */

#include "TextureView.hpp"

#include <Graphics/Common/TextureMemory.hpp>

namespace KryneEngine
{
    void TextureView::_SetMemory(TextureMemory *_memory)
    {
        if (m_memory != nullptr)
        {
            m_memory->_RemoveView();
        }

        m_memory = _memory;

        if (m_memory != nullptr)
        {
            m_memory->_AddView();
        }
    }
}