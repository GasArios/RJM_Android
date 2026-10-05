#pragma once

// PlayerHitResolver.h
// - 적 접촉 공격과 플레이어 Hurtbox 사이의 충돌을 해석합니다.
// - Player는 "피해를 받으면 어떻게 반응하는가"를 담당하고,
//   이 resolver는 "무엇이 플레이어를 맞혔는가"만 판단합니다.

#include "RecoilJumpMan/Combat/HitEvent.h"

#include <vector>

namespace rjm
{
    class EnemyPool;
    class Player;

    class PlayerHitResolver
    {
    public:
        // ResolveEnemyContact:
        // - 활성 적의 접촉 공격 판정이 플레이어 Hurtbox와 겹치는지 확인합니다.
        // - 실제 피해 적용은 Player::ApplyDamage에 맡기고, 성공한 경우에만 HitEvent를 만듭니다.
        void ResolveEnemyContact(
            Player& player,
            const EnemyPool& enemies,
            double currentTimeSeconds,
            std::vector<HitEvent>& outHitEvents) const;

    private:
        static bool RectanglesOverlap(Rectangle a, Rectangle b);
        static Vector2 HitNormal(Vector2 hitSourcePosition, Vector2 targetPosition);
        static WeaponFeedbackProfile BuildPlayerHitFeedback();
    };
}

