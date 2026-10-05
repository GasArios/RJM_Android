// PlayerHitResolver.cpp
// - 플레이어가 적 접촉 공격에 맞았는지 확인하고 피격 이벤트를 생성합니다.

#include "RecoilJumpMan/Combat/PlayerHitResolver.h"

#include "RecoilJumpMan/Entity/Enemy.h"
#include "RecoilJumpMan/Entity/EnemyPool.h"
#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/Player/Player.h"

namespace rjm
{
    void PlayerHitResolver::ResolveEnemyContact(
        Player& player,
        const EnemyPool& enemies,
        double,
        std::vector<HitEvent>& outHitEvents) const
    {
        if (!player.CanReceiveDamage())
        {
            return;
        }

        const Rectangle playerHurtbox = player.Hurtbox();
        const std::vector<Enemy>& enemyList = enemies.Enemies();

        for (std::size_t enemyIndex : enemies.ActiveIndices())
        {
            if (enemyIndex >= enemyList.size())
            {
                continue;
            }

            const Enemy& enemy = enemyList[enemyIndex];
            if (!enemy.CanDamagePlayerOnContact())
            {
                continue;
            }

            if (!RectanglesOverlap(playerHurtbox, enemy.ContactHitbox()))
            {
                continue;
            }

            const Damage damage = enemy.ContactDamage();
            const Vector2 hitNormal = HitNormal(enemy.Position(), player.Position());
            const DamageResult damageResult = player.ApplyDamage(damage, hitNormal);
            if (!damageResult.accepted)
            {
                return;
            }

            HitEvent event;
            event.sourceId = enemy.Definition().id;
            event.targetId = "player";
            event.hitEffectId = "player_hit";
            event.position = player.Position();
            event.normal = hitNormal;
            event.damage = damage;
            event.damageResult = damageResult;
            event.feedback = BuildPlayerHitFeedback();
            outHitEvents.push_back(event);

            // 플레이어 피격은 무적 시간을 시작하므로, 같은 프레임에 여러 적이 겹쳐도 한 번만 처리합니다.
            return;
        }
    }

    bool PlayerHitResolver::RectanglesOverlap(Rectangle a, Rectangle b)
    {
        return a.x < b.x + b.width
            && a.x + a.width > b.x
            && a.y < b.y + b.height
            && a.y + a.height > b.y;
    }

    Vector2 PlayerHitResolver::HitNormal(Vector2 hitSourcePosition, Vector2 targetPosition)
    {
        return math::NormalizeOr(
            math::Subtract(targetPosition, hitSourcePosition),
            { 0.0f, 1.0f });
    }

    WeaponFeedbackProfile PlayerHitResolver::BuildPlayerHitFeedback()
    {
        WeaponFeedbackProfile feedback;
        feedback.hitStopSeconds = 0.055f;
        feedback.hitShakeStrength = 9.0f;
        feedback.hitShakeSeconds = 0.14f;
        feedback.hitShakeFrequency = 28.0f;
        return feedback;
    }
}

