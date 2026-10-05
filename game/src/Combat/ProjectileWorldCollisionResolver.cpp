#include "RecoilJumpMan/Combat/ProjectileWorldCollisionResolver.h"

#include "RecoilJumpMan/Combat/Projectile.h"
#include "RecoilJumpMan/Combat/ProjectilePool.h"
#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/World/TileMap.h"

#include <algorithm>
#include <cmath>
#include <vector>

namespace rjm
{
    void ProjectileWorldCollisionResolver::ResolveProjectilesAgainstWorld(
        ProjectilePool& projectiles,
        const TileMap& tileMap) const
    {
        for (Projectile& projectile : projectiles.Projectiles())
        {
            if (!projectile.IsActive())
            {
                continue;
            }

            const ProjectileWorldHit hit = TraceProjectilePath(
                tileMap,
                projectile.PreviousPosition(),
                projectile.Position(),
                projectile.HitRadius());

            if (!hit.hit)
            {
                continue;
            }

            projectile.SetPosition(hit.position);

            // 벽 관통은 적 관통/다단 히트와 별도 규칙으로 다루는 편이 안전합니다.
            // 현재 모든 투사체는 solid 타일에 닿으면 즉시 사라집니다.
            projectile.SetActive(false);
        }
    }

    ProjectileWorldHit ProjectileWorldCollisionResolver::TraceProjectilePath(
        const TileMap& tileMap,
        Vector2 fromWorld,
        Vector2 toWorld,
        float radius)
    {
        ProjectileWorldHit result;
        result.position = toWorld;

        const float safeRadius = std::max(0.0f, radius);
        const Vector2 delta = math::Subtract(toWorld, fromWorld);
        const float distance = math::Length(delta);
        const float sampleSpacing = std::max(1.0f, safeRadius);
        const int sampleCount = distance > math::VectorEpsilon
            ? std::max(1, static_cast<int>(std::ceil(distance / sampleSpacing)))
            : 1;

        std::vector<Rectangle> solidTiles;
        Vector2 previousOpenPoint = fromWorld;

        for (int sample = 0; sample <= sampleCount; ++sample)
        {
            const float t = static_cast<float>(sample) / static_cast<float>(sampleCount);
            const Vector2 point = {
                fromWorld.x + delta.x * t,
                fromWorld.y + delta.y * t
            };

            const Rectangle queryBounds = {
                point.x - safeRadius,
                point.y - safeRadius,
                safeRadius * 2.0f,
                safeRadius * 2.0f
            };

            tileMap.CollectSolidTiles(queryBounds, solidTiles);
            for (const Rectangle& solidTile : solidTiles)
            {
                if (CircleIntersectsRect(point, safeRadius, solidTile))
                {
                    result.hit = true;
                    result.position = previousOpenPoint;
                    return result;
                }
            }

            previousOpenPoint = point;
        }

        return result;
    }

    bool ProjectileWorldCollisionResolver::CircleIntersectsRect(Vector2 center, float radius, Rectangle rect)
    {
        const float nearestX = std::clamp(center.x, rect.x, rect.x + rect.width);
        const float nearestY = std::clamp(center.y, rect.y, rect.y + rect.height);
        const float dx = center.x - nearestX;
        const float dy = center.y - nearestY;
        return dx * dx + dy * dy <= radius * radius;
    }
}

