//
// Created by Zach Lee on 2022/7/30.
//

#pragma once
#include <core/math/Matrix.h>
#include <core/math/Math.h>

namespace sky {

    struct MathUtil {
        static void PrintMatrix(const Matrix4& m);
    };

    inline float ToRadian(float value)
    {
        return value / 180.f * PI;
    }
}
