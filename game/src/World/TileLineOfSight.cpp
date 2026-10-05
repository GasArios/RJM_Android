#include "RecoilJumpMan/World/TileLineOfSight.h"

#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/World/TileMap.h"

#include <algorithm>
#include <cmath>

namespace rjm
{
    TileLineOfSightResult TileLineOfSight::Trace(const TileMap& tileMap, Vector2 fromWorld, Vector2 toWorld)
    {
        TileLineOfSightResult result;
        result.visibleEndWorld = toWorld;

        const Vector2 delta = math::Subtract(toWorld, fromWorld);
        const float distance = math::Length(delta);
        if (distance <= math::VectorEpsilon)
        {
            return result;
        }

        const float stepLength = std::max(4.0f, static_cast<float>(tileMap.TileSize()) * 0.5f);
        const int sampleCount = std::max(1, static_cast<int>(std::ceil(distance / stepLength)));

        // 시작점과 끝점 자체는 검사하지 않습니다.
        // 플레이어/적 중심이 타일 경계에 살짝 걸쳐도 중간 경로가 열려 있으면 시야선이 있다고 봅니다.
        Vector2 previous = fromWorld;
        for (int sample = 1; sample < sampleCount; ++sample)
        {
            const float t = static_cast<float>(sample) / static_cast<float>(sampleCount);
            const Vector2 point = {
                fromWorld.x + delta.x * t,
                fromWorld.y + delta.y * t
            };

            if (tileMap.IsSolidAtWorld(point))
            {
                result.hasLineOfSight = false;
                result.visibleEndWorld = previous;
                return result;
            }

            previous = point;
        }

        return result;
    }
}
