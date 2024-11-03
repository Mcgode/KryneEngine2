/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#pragma once

#include <Graphics/Metal/MetalHeaders.hpp>

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

        void ResetEncoder()
        {
            if (m_encoder != nullptr)
            {
                m_encoder->endEncoding();
                m_encoder.reset();
            }
        }

        void ResetEncoder(EncoderType _type)
        {
            if (m_encoder != nullptr && m_type != _type)
            {
                m_encoder->endEncoding();
                m_encoder.reset();
            }
            m_type = _type;
        }
    };

    using CommandList = CommandListData*;
}
