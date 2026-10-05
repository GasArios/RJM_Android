// Projectile.cpp
// - Projectile 클래스의 구현부입니다.
// - 현재는 단순히 직선 이동하고, 일정 시간이 지나면 비활성화됩니다.

#include "RecoilJumpMan/Combat/Projectile.h"

#include "RecoilJumpMan/Physics/CoordinateSpace.h"

#include <raylib.h>

#include <algorithm>
#include <utility>

namespace rjm
{
    Projectile::Projectile()
    {
        SetActive(false);
    }

    // 생성자:
    // - damage_는 멤버 초기화 리스트로 초기화합니다.
    // - position_과 velocity_는 Entity에서 물려받은 protected 멤버입니다.
    Projectile::Projectile(Vector2 position, Vector2 velocity, Damage damage)
    {
        Reset(position, velocity, damage);
    }

    Projectile::Projectile(
        Vector2 position,
        Vector2 velocity,
        ProjectileHitPayload payload,
        DefinitionId sourceId,
        float lifetimeSeconds,
        float hitRadius)
    {
        Reset(position, velocity, payload, sourceId, lifetimeSeconds, hitRadius);
    }

    // Reset:
    // - Damage만 받는 예전 경로입니다.
    // - 내부적으로는 ProjectileHitPayload를 만들어 새 경로로 넘기므로,
    //   예전 호출부와 새 명중 피드백/관통/이펙트 구조가 같은 초기화 코드를 공유합니다.
    void Projectile::Reset(Vector2 position, Vector2 velocity, Damage damage)
    {
        // 기존 코드 경로는 Damage만 넘겨도 투사체를 만들 수 있게 유지합니다.
        // 내부적으로는 새 구조인 ProjectileHitPayload로 감싸서 같은 Reset 흐름을 사용합니다.
        ProjectileHitPayload payload;
        payload.damage = damage;
        Reset(position, velocity, payload);
    }

    void Projectile::Reset(
        Vector2 position,
        Vector2 velocity,
        ProjectileHitPayload payload,
        DefinitionId sourceId,
        float lifetimeSeconds,
        float hitRadius)
    {
        // position_과 velocity_는 Entity의 중심 좌표/속도 규칙을 그대로 따릅니다.
        position_ = position;
        previousPosition_ = position;
        velocity_ = velocity;

        // payload는 이 투사체가 명중했을 때 CombatHitResolver가 읽을 전투 정보입니다.
        hitPayload_ = std::move(payload);
        sourceId_ = std::move(sourceId);

        // 관통 횟수는 발사 시 payload 값으로 초기화합니다.
        // 명중할 때마다 ResolveHitLifetime에서 하나씩 줄어듭니다.
        remainingPierceCount_ = hitPayload_.pierceCount;

        // lifetimeSeconds와 hitRadius는 무기 데이터에서 내려옵니다.
        // 샷건 펠릿은 짧은 lifetime으로 사거리를 제한하고,
        // 핸드캐논 같은 큰 탄은 hitRadius를 키워 묵직한 판정을 줄 수 있습니다.
        lifetimeSeconds_ = std::max(0.01f, lifetimeSeconds);
        hitRadius_ = std::max(0.5f, hitRadius);
        SetActive(true);
    }

    // Update:
    // - 속도 * 시간만큼 위치를 이동합니다.
    // - lifetimeSeconds_가 0 이하가 되면 비활성화합니다.
    void Projectile::Update(float deltaSeconds)
    {
        previousPosition_ = position_;
        position_.x += velocity_.x * deltaSeconds;
        position_.y += velocity_.y * deltaSeconds;
        lifetimeSeconds_ -= deltaSeconds;

        if (lifetimeSeconds_ <= 0.0f)
        {
            SetActive(false);
        }
    }

    // Draw:
    // - 현재는 작은 노란 원으로 총알을 표시합니다.
    void Projectile::Draw() const
    {
        // Projectile도 내부 위치는 y-up 물리 좌표입니다.
        // BeginMode2D 안에서 그릴 것이므로 Raylib 렌더 좌표로만 변환합니다.
        // 디버그 단계에서는 실제 HitRadius와 같은 크기로 그려 판정 크기 튜닝을 바로 볼 수 있게 합니다.
        DrawCircleV(CoordinateSpace::WorldToRender(position_), hitRadius_, Color{ 255, 236, 128, 255 });
    }

    // DamagePayload:
    // - 피해 정보를 const 참조로 반환합니다.
    // - 복사 비용을 줄이고, 외부에서 수정하지 못하게 const를 붙였습니다.
    const Damage& Projectile::DamagePayload() const
    {
        return hitPayload_.damage;
    }

    const ProjectileHitPayload& Projectile::HitPayload() const
    {
        return hitPayload_;
    }

    const DefinitionId& Projectile::SourceId() const
    {
        return sourceId_;
    }

    float Projectile::HitRadius() const
    {
        return hitRadius_;
    }

    Vector2 Projectile::PreviousPosition() const
    {
        return previousPosition_;
    }

    void Projectile::ResolveHitLifetime()
    {
        // destroyOnHit가 false인 투사체는 명중해도 여기서 사라지지 않습니다.
        // 빔, 장판, 지속 피해 영역 같은 특수 투사체를 위한 여지입니다.
        if (!hitPayload_.destroyOnHit)
        {
            return;
        }

        // 관통 횟수가 남아 있으면 이번 명중에서는 살아남습니다.
        if (remainingPierceCount_ > 0)
        {
            --remainingPierceCount_;
            return;
        }

        // 일반 탄환은 첫 명중 후 비활성화되어 ProjectilePool에서 재사용될 수 있습니다.
        SetActive(false);
    }
}

