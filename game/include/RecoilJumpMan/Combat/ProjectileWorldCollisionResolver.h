#pragma once

// ProjectileWorldCollisionResolver.h
// - 활성 투사체와 solid 타일 사이의 충돌을 처리합니다.
// - 적 충돌보다 먼저 실행해 벽 뒤의 적에게 총알이 닿지 않게 합니다.

#include <raylib.h>

namespace rjm
{
    class ProjectilePool;
    class TileMap;

    struct ProjectileWorldHit
    {
        bool hit = false;
        Vector2 position = { 0.0f, 0.0f };
    };

    class ProjectileWorldCollisionResolver
    {
    public:
        void ResolveProjectilesAgainstWorld(ProjectilePool& projectiles, const TileMap& tileMap) const;

    private:
        static ProjectileWorldHit TraceProjectilePath(
            const TileMap& tileMap,
            Vector2 fromWorld,
            Vector2 toWorld,
            float radius);

        static bool CircleIntersectsRect(Vector2 center, float radius, Rectangle rect);
    };
}

