/**
 * @file
 * @author Max Godefroy
 * @date 29/10/2024.
 */

#pragma once

#include "Graphics/Metal/MetalHeaders.hpp"
#include "KryneEngine/Core/Common/Assert.hpp"

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

        MTL::CommandBuffer* m_commandBuffer { nullptr };
        NsPtr<MTL::CommandEncoder> m_encoder { nullptr };
        EncoderType m_type = EncoderType::Render;
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

    struct TimestampConversion
    {
        double m_gpuFrequency = 0.f;
        u64 m_gpuReference = 0;
        u64 m_cpuReference = 0;

        [[nodiscard]] u64 ConvertGpuTimestamp(u64 _gpuTimestamp) const
        {
            return m_cpuReference + static_cast<u64>(static_cast<double>(_gpuTimestamp - m_gpuReference) * m_gpuFrequency);
        }
    };
}
