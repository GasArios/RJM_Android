#pragma once

// Projectile.h
// - 총알, 탄환, 마법탄 같은 날아가는 공격체를 표현하는 클래스입니다.
// - Entity를 상속하므로 위치, 속도, 활성 상태를 가집니다.

#include "RecoilJumpMan/Combat/ProjectileHitPayload.h"
#include "RecoilJumpMan/Entity/Entity.h"

namespace rjm
{
    class Projectile final : public Entity
    {
    public:
        // 기본 생성자:
        // - ProjectilePool에서 미리 여러 개 만들어두기 위해 필요합니다.
        Projectile();

        // 생성자:
        // - 시작 위치, 속도, 피해 정보를 받아 투사체를 만듭니다.
        Projectile(Vector2 position, Vector2 velocity, Damage damage);

        // 생성자:
        // - 피해량뿐 아니라 명중 피드백, 이펙트 id, 관통/폭발 규칙까지 포함한 payload로 투사체를 만듭니다.
        // - sourceId는 이 투사체를 만든 무기/스킬/오브젝트의 id입니다.
        // - lifetimeSeconds와 hitRadius는 무기별 투사체 사거리/판정 크기를 표현합니다.
        //   예: 샷건 펠릿은 짧은 수명, 핸드캐논은 큰 반지름을 줄 수 있습니다.
        Projectile(
            Vector2 position,
            Vector2 velocity,
            ProjectileHitPayload payload,
            DefinitionId sourceId = {},
            float lifetimeSeconds = 2.0f,
            float hitRadius = 4.0f);

        // Reset:
        // - 풀에 있던 비활성 투사체를 새 총알처럼 재사용합니다.
        void Reset(Vector2 position, Vector2 velocity, Damage damage);

        // Reset:
        // - ProjectileHitPayload를 사용해 풀에 있던 투사체를 새 발사체 상태로 되돌립니다.
        // - ProjectilePool은 new/delete 없이 이 함수를 호출해 투사체를 반복 재사용합니다.
        // - lifetimeSeconds와 hitRadius를 매번 받기 때문에 같은 Projectile 객체를
        //   리볼버 탄, 샷건 펠릿, 핸드캐논 탄처럼 서로 다른 성격으로 재사용할 수 있습니다.
        void Reset(
            Vector2 position,
            Vector2 velocity,
            ProjectileHitPayload payload,
            DefinitionId sourceId = {},
            float lifetimeSeconds = 2.0f,
            float hitRadius = 4.0f);

        // Update:
        // - 매 프레임 위치를 이동하고 수명을 줄입니다.
        void Update(float deltaSeconds) override;

        // Draw:
        // - 투사체를 화면에 그립니다.
        void Draw() const override;

        // DamagePayload:
        // - 이 투사체가 충돌했을 때 전달할 피해 정보를 반환합니다.
        // - 예전 코드와의 호환을 위해 남겨둔 읽기 함수입니다.
        const Damage& DamagePayload() const;

        // HitPayload:
        // - 피해, 피격 피드백, 이펙트 id, 관통/폭발 예약값을 포함한 전체 명중 정보를 반환합니다.
        const ProjectileHitPayload& HitPayload() const;

        // SourceId:
        // - 이 투사체를 만든 데이터 id를 반환합니다.
        // - 현재는 무기 id를 넣고, 나중에는 스킬/함정/적 탄환 id도 사용할 수 있습니다.
        const DefinitionId& SourceId() const;

        // HitRadius:
        // - 충돌 판정에 사용할 투사체 반지름입니다.
        // - 현재 총알은 작은 원으로 처리하지만, 무기 데이터에 따라 탄마다 크기를 다르게 줄 수 있습니다.
        float HitRadius() const;

        // PreviousPosition:
        // - 직전 Update가 시작될 때의 위치입니다.
        // - 빠른 투사체가 프레임 사이에 벽을 건너뛰지 않도록 이동 구간 충돌에 사용합니다.
        Vector2 PreviousPosition() const;

        // ResolveHitLifetime:
        // - 명중 후 투사체가 사라질지, 관통 수를 하나 줄이고 계속 날아갈지 처리합니다.
        void ResolveHitLifetime();

    private:
        // 명중 시 전달할 전투 정보 묶음입니다.
        ProjectileHitPayload hitPayload_;

        // 발사 주체 또는 발사 무기의 데이터 id입니다.
        DefinitionId sourceId_;

        // 남은 관통 횟수입니다.
        // 0이면 다음 명중에서 일반 탄처럼 사라집니다.
        int remainingPierceCount_ = 0;

        // 직전 이동 시작 위치입니다.
        Vector2 previousPosition_ = { 0.0f, 0.0f };

        // 투사체 충돌 반지름입니다.
        // 디버그 렌더링도 이 값을 사용하므로 화면에 보이는 탄 크기와 판정 크기가 일치합니다.
        float hitRadius_ = 4.0f;

        // 투사체가 살아있는 남은 시간입니다.
        // 사거리는 보통 WeaponDefinition의 projectileRange / bulletSpeed로 환산되어 들어옵니다.
        float lifetimeSeconds_ = 2.0f;
    };
}

