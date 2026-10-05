// ProjectilePool.cpp
// - 고정 크기 투사체 풀 구현부입니다.

#include "RecoilJumpMan/Combat/ProjectilePool.h"

#include <utility>

namespace rjm
{
    void ProjectilePool::Initialize(std::size_t capacity)
    {
        projectiles_.clear();
        projectiles_.resize(capacity);
    }

    Projectile* ProjectilePool::Spawn(Vector2 position, Vector2 velocity, Damage damage)
    {
        // 예전 호출자는 Damage만 넘겨도 사용할 수 있게 유지합니다.
        // 새 전투 흐름에서는 아래 payload 버전 Spawn을 사용하는 것이 더 확장성이 좋습니다.
        ProjectileHitPayload payload;
        payload.damage = damage;
        return Spawn(position, velocity, payload);
    }

    Projectile* ProjectilePool::Spawn(
        Vector2 position,
        Vector2 velocity,
        ProjectileHitPayload payload,
        DefinitionId sourceId,
        float lifetimeSeconds,
        float hitRadius)
    {
        // 비활성 투사체를 찾아 새 탄환으로 재사용합니다.
        // 풀 안에 빈 칸이 없으면 이번 발사는 생성하지 않고 nullptr를 반환합니다.
        // 샷건처럼 한 번에 여러 투사체를 만들 때도 이 함수가 반복 호출되므로,
        // new/delete 없이 같은 풀을 재사용하는 구조를 유지합니다.
        for (Projectile& projectile : projectiles_)
        {
            if (!projectile.IsActive())
            {
                projectile.Reset(
                    position,
                    velocity,
                    std::move(payload),
                    std::move(sourceId),
                    lifetimeSeconds,
                    hitRadius);
                return &projectile;
            }
        }

        return nullptr;
    }

    void ProjectilePool::Update(float deltaSeconds)
    {
        for (Projectile& projectile : projectiles_)
        {
            if (projectile.IsActive())
            {
                projectile.Update(deltaSeconds);
            }
        }
    }

    void ProjectilePool::Draw() const
    {
        for (const Projectile& projectile : projectiles_)
        {
            if (projectile.IsActive())
            {
                projectile.Draw();
            }
        }
    }

    std::vector<Projectile>& ProjectilePool::Projectiles()
    {
        // 현재는 CombatHitResolver가 직접 순회할 수 있게 vector 참조를 반환합니다.
        // 소유권은 여전히 ProjectilePool이 가지고 있습니다.
        return projectiles_;
    }

    const std::vector<Projectile>& ProjectilePool::Projectiles() const
    {
        return projectiles_;
    }

    std::size_t ProjectilePool::ActiveCount() const
    {
        std::size_t count = 0;
        for (const Projectile& projectile : projectiles_)
        {
            if (projectile.IsActive())
            {
                ++count;
            }
        }

        return count;
    }

    std::size_t ProjectilePool::Capacity() const
    {
        return projectiles_.size();
    }
}

