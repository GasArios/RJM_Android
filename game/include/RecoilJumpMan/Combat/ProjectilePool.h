#pragma once

// ProjectilePool.h
// - 발사할 때마다 new/delete하지 않고, 미리 만들어둔 Projectile을 재사용합니다.

#include "RecoilJumpMan/Combat/Projectile.h"

#include <cstddef>
#include <vector>

namespace rjm
{
    class ProjectilePool
    {
    public:
        void Initialize(std::size_t capacity);
        Projectile* Spawn(Vector2 position, Vector2 velocity, Damage damage);

        // Spawn:
        // - ProjectileHitPayload를 가진 투사체를 풀에서 꺼내 활성화합니다.
        // - 명중 피드백, 이펙트 id, 관통/폭발 규칙이 필요한 새 전투 흐름에서 사용합니다.
        // - lifetimeSeconds/hitRadius는 ShotProjectile에서 내려온 값입니다.
        //   그래서 한 발 발사 결과에 리볼버 탄 1개 또는 샷건 펠릿 여러 개가 있어도 같은 Spawn 경로를 씁니다.
        Projectile* Spawn(
            Vector2 position,
            Vector2 velocity,
            ProjectileHitPayload payload,
            DefinitionId sourceId = {},
            float lifetimeSeconds = 2.0f,
            float hitRadius = 4.0f);
        void Update(float deltaSeconds);
        void Draw() const;

        // Projectiles:
        // - CombatHitResolver가 활성 투사체를 검사할 수 있도록 내부 풀을 반환합니다.
        // - 나중에 성능 최적화가 필요하면 이 함수 대신 충돌 후보 query API로 바꿀 수 있습니다.
        std::vector<Projectile>& Projectiles();
        const std::vector<Projectile>& Projectiles() const;

        std::size_t ActiveCount() const;
        std::size_t Capacity() const;

    private:
        std::vector<Projectile> projectiles_;
    };
}

