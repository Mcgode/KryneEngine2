/**
 * @file
 * @author Max Godefroy
 * @date 13/03/2025.
 */

#include <KryneEngine/Core/Common/Types.hpp>
#include <KryneEngine/Core/Math/Vector.hpp>
#include <KryneEngine/Core/Math/RotationConversion.hpp>
#include <gtest/gtest.h>

#include "Utils/AssertUtils.hpp"

using namespace KryneEngine::Math;

namespace KryneEngine::Tests::Math
{
    template <EulerOrder Order>
    void PerformBasicEulerToQuaternionTest()
    {
        {
            // Identity quaternion should be equal to null euler
            const Quaternion q = FromEulerAngles<float, float, 4, Order>(float3(0));
            EXPECT_EQ(Quaternion(), q);
        }

        {
            // Rotation around the X axis
            constexpr float angle = M_PI_2 * 0.8f;
            const Quaternion expected = Quaternion().FromAxisAngle(float3(1, 0, 0), angle);
            const Quaternion result = FromEulerAngles<float, float, 4, Order>(float3(angle, 0, 0));
            EXPECT_EQ(expected, result);
        }

        {
            // Rotation around the Y axis
            constexpr float angle = M_PI_2 * 0.6f;
            const Quaternion expected = Quaternion().FromAxisAngle(float3(0, 1, 0), angle);
            const Quaternion result = FromEulerAngles<float, float, 4, Order>(float3(0, angle, 0));
            EXPECT_EQ(expected, result);
        }

        {
            // Rotation around the Z axis
            constexpr float angle = M_PI_2 * 0.2f;
            const Quaternion expected = Quaternion().FromAxisAngle(float3(0, 0, 1), angle);
            const Quaternion result = FromEulerAngles<float, float, 4, Order>(float3(0, 0, angle));
            EXPECT_EQ(expected, result);
        }
    }

    TEST(RotationConversion, BasicEulerToQuaternion)
    {
        // -----------------------------------------------------------------------
        // Execute
        // -----------------------------------------------------------------------

        PerformBasicEulerToQuaternionTest<EulerOrder::XYZ>();
        PerformBasicEulerToQuaternionTest<EulerOrder::XZY>();
        PerformBasicEulerToQuaternionTest<EulerOrder::YXZ>();
        PerformBasicEulerToQuaternionTest<EulerOrder::YZX>();
        PerformBasicEulerToQuaternionTest<EulerOrder::ZXY>();
        PerformBasicEulerToQuaternionTest<EulerOrder::ZYX>();
    }
}