/**
 * @file
 * @author Max Godefroy
 * @date 22/03/2022.
 */

#pragma once

#include "KryneEngine/Core/Common/Types.hpp"

namespace KryneEngine::VkCommonStructures
{
    struct QueueIndices
    {
        static constexpr s32 kInvalid = -1;
        struct Pair {
            s32 m_familyIndex = kInvalid;
            s32 m_indexInFamily = kInvalid;

            [[nodiscard]] bool IsInvalid() const
            {
                return m_familyIndex == kInvalid || m_indexInFamily == kInvalid;
            }
        };

        Pair m_graphicsQueueIndex {};
        Pair m_transferQueueIndex {};
        Pair m_computeQueueIndex {};
        Pair m_presentQueueIndex {};

        [[nodiscard]] eastl::vector<u32> RetrieveDifferentFamilies() const
        {
            eastl::vector<u32> differentFamilyIndices;

            const auto tryAppending = [&differentFamilyIndices](const Pair& _pair)
            {
                if (_pair.IsInvalid())
                {
                    return;
                }

                const auto isAlreadyInserted =
                        eastl::find(differentFamilyIndices.begin(), differentFamilyIndices.end(), _pair.m_familyIndex)
                        != differentFamilyIndices.end();
                if (!isAlreadyInserted)
                {
                    differentFamilyIndices.push_back(_pair.m_familyIndex);
                }
            };

            tryAppending(m_graphicsQueueIndex);
            tryAppending(m_transferQueueIndex);
            tryAppending(m_computeQueueIndex);
            tryAppending(m_presentQueueIndex);

            return differentFamilyIndices;
        }
    };
}
