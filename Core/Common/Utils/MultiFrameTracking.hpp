/**
 * @file
 * @author Max Godefroy
 * @date 28/07/2024.
 */

#pragma once

#include <Common/Arrays.hpp>
#include <EASTL/span.h>

namespace KryneEngine
{
    template <class UserData>
    class MultiFrameDataTracker
    {
    public:
        inline void Init(u8 _frameCount, u8 _frameIndex)
        {
            m_currentFrame = _frameIndex;
            m_trackedData.Resize(_frameCount);
            m_trackedData.InitAll();
        }

        inline void AdvanceToNextFrame()
        {
            // Advance frame index
            m_currentFrame = (m_currentFrame + 1) % m_trackedData.Size();
        }

        inline void ClearData()
        {
            // Clear current frame
            GetData(0).clear();
        }

        inline void TrackForOtherFrames(const UserData& _userData)
        {
            for (u8 i = 1; i < m_trackedData.Size(); i++)
            {
                GetData(i).push_back(_userData);
            }
        }

        inline const eastl::vector<UserData>& GetData()
        {
            return GetData(0);
        }

    private:
        u8 m_currentFrame;

        DynamicArray<eastl::vector<UserData>> m_trackedData;

        inline eastl::vector<UserData>& GetData(u8 _offset)
        {
            const u8 index = (m_currentFrame + _offset) % m_trackedData.Size();
            return m_trackedData[index];
        }
    };
}