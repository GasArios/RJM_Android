#pragma once

// VectorMath.h
// - Raylib Vector2에 대한 작은 공통 수학 함수 모음입니다.
// - 각 시스템이 Length/Distance/Normalize를 따로 구현하지 않도록 한곳에 둡니다.

#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace rjm::math
{
    constexpr float Pi = 3.1415926535f;
    constexpr float VectorEpsilon = 0.0001f;

    inline Vector2 Add(Vector2 a, Vector2 b)
    {
        return { a.x + b.x, a.y + b.y };
    }

    inline Vector2 Subtract(Vector2 a, Vector2 b)
    {
        return { a.x - b.x, a.y - b.y };
    }

    inline Vector2 Scale(Vector2 value, float scale)
    {
        return { value.x * scale, value.y * scale };
    }

    inline float LengthSquared(Vector2 value)
    {
        return value.x * value.x + value.y * value.y;
    }

    inline float Length(Vector2 value)
    {
        return std::sqrt(LengthSquared(value));
    }

    inline float DistanceSquared(Vector2 a, Vector2 b)
    {
        return LengthSquared(Subtract(a, b));
    }

    inline float Distance(Vector2 a, Vector2 b)
    {
        return std::sqrt(DistanceSquared(a, b));
    }

    inline Vector2 NormalizeOr(Vector2 value, Vector2 fallback)
    {
        const float length = Length(value);
        if (length <= VectorEpsilon)
        {
            return fallback;
        }

        return { value.x / length, value.y / length };
    }

    inline Vector2 NormalizeOrZero(Vector2 value)
    {
        return NormalizeOr(value, { 0.0f, 0.0f });
    }

    inline Vector2 NormalizeOrRight(Vector2 value)
    {
        return NormalizeOr(value, { 1.0f, 0.0f });
    }

    inline float DirectionDifferenceDegrees(Vector2 fromA, Vector2 toA, Vector2 fromB, Vector2 toB)
    {
        const Vector2 a = Subtract(toA, fromA);
        const Vector2 b = Subtract(toB, fromB);

        const float lengthA = Length(a);
        const float lengthB = Length(b);
        if (lengthA <= VectorEpsilon || lengthB <= VectorEpsilon)
        {
            return 0.0f;
        }

        const float dot = (a.x * b.x + a.y * b.y) / (lengthA * lengthB);
        const float clampedDot = std::clamp(dot, -1.0f, 1.0f);
        return std::acos(clampedDot) * 180.0f / Pi;
    }
}

