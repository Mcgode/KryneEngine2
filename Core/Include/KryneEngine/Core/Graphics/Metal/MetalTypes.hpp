/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#pragma once

#include "KryneEngine/Core/Common/Assert.hpp"
#include "KryneEngine/Core/Graphics/Metal/MetalHeaders.hpp"

namespace KryneEngine
{

    struct CommandListData
    {
        enum class EncoderType
        {
            Render,
            Blit,
            Compute
        };

        MTL::CommandBuffer* m_commandBuffer;
        NsPtr<MTL::CommandEncoder> m_encoder;
        EncoderType m_type;
        void* m_userData = nullptr;

        void ResetEncoder()
        {
            if (m_encoder != nullptr)
            {
                KE_ASSERT(m_userData == nullptr);
                m_encoder->endEncoding();
                m_encoder.reset();
            }
        }

        void ResetEncoder(EncoderType _type)
        {
            if (m_encoder != nullptr && m_type != _type)
            {
                KE_ASSERT(m_userData == nullptr);
                m_encoder->endEncoding();
                m_encoder.reset();
            }
            m_type = _type;
        }
    };

    using CommandList = CommandListData*;
}
