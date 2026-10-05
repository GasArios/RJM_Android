#pragma once

// CombatHitResolver.h
// - 투사체와 적의 충돌을 해석하고 HitEvent를 만들어내는 전투 판정기입니다.
// - 이 클래스는 "맞았는가, 피해가 적용되었는가"만 판단합니다.
// - 피격 이펙트, 사운드, 카메라 흔들림은 HitEvent를 받은 다른 시스템이 처리합니다.

#include "RecoilJumpMan/Combat/HitEvent.h"

#include <vector>

namespace rjm
{
    class EnemyPool;
    class ProjectilePool;

    class CombatHitResolver
    {
    public:
        // ResolveProjectilesAgainstEnemies:
        // - 활성 투사체와 활성 적을 순회하며 충돌을 검사합니다.
        // - 충돌이 성립하면 Enemy::ApplyDamage로 피해를 적용하고,
        //   그 결과를 HitEvent로 만들어 outHitEvents에 추가합니다.
        // - 이 함수는 이펙트나 사운드를 직접 만들지 않습니다.
        //   GameplayScene 같은 호출자가 HitEvent를 보고 필요한 연출 시스템에 전달합니다.
        void ResolveProjectilesAgainstEnemies(
            ProjectilePool& projectiles,
            EnemyPool& enemies,
            double currentTimeSeconds,
            std::vector<HitEvent>& outHitEvents) const;

    private:
        // CircleIntersectsRect:
        // - 현재 투사체는 작은 원, 적은 사각형 Hurtbox로 취급합니다.
        // - 원 중심에서 사각형의 가장 가까운 점까지의 거리가 반지름 이하이면 충돌입니다.
        static bool CircleIntersectsRect(Vector2 center, float radius, Rectangle rect);

        // HitNormal:
        // - 피격 위치에서 대상 중심을 바라본 방향의 반대쪽 단위 벡터를 구합니다.
        // - 피격 파티클이 어느 방향으로 튈지, 넉백이 어느 방향인지 같은 연출/물리에 사용할 수 있습니다.
        static Vector2 HitNormal(Vector2 hitPosition, Vector2 targetPosition);
    };
}

