// CombatHitResolver.cpp
// - 투사체와 적 사이의 충돌 판정, 피해 적용, HitEvent 생성을 담당합니다.
// - 이 파일은 전투 결과를 만들 뿐이며, 피격 연출은 다른 시스템에 맡깁니다.

#include "RecoilJumpMan/Combat/CombatHitResolver.h"

#include "RecoilJumpMan/Combat/Projectile.h"
#include "RecoilJumpMan/Combat/ProjectilePool.h"
#include "RecoilJumpMan/Entity/Enemy.h"
#include "RecoilJumpMan/Entity/EnemyPool.h"
#include "RecoilJumpMan/Math/VectorMath.h"

#include <algorithm>

namespace rjm
{
    void CombatHitResolver::ResolveProjectilesAgainstEnemies(
        ProjectilePool& projectiles,
        EnemyPool& enemies,
        double currentTimeSeconds,
        std::vector<HitEvent>& outHitEvents) const
    {
        // 현재는 활성 투사체 x 활성 적의 단순 이중 루프입니다.
        // EnemyPool이 플레이어 주변 적만 active로 유지하므로, 프로토타입 단계에서는 충분히 가볍습니다.
        // 나중에 탄막/다수 몬스터가 많아지면 이 함수 내부를 공간 분할 쿼리로 바꾸면 됩니다.
        for (Projectile& projectile : projectiles.Projectiles())
        {
            if (!projectile.IsActive())
            {
                continue;
            }

            for (std::size_t enemyIndex : enemies.ActiveIndices())
            {
                // 관통하지 않는 투사체는 첫 명중에서 비활성화됩니다.
                // 그 경우 남은 적을 계속 검사할 이유가 없으므로 안쪽 루프를 종료합니다.
                if (!projectile.IsActive())
                {
                    break;
                }

                Enemy& enemy = enemies.Enemies()[enemyIndex];
                if (!enemy.IsActive() || !enemy.IsAlive())
                {
                    continue;
                }

                // 현재 충돌 모델:
                // - 투사체: 작은 원
                // - 적: Enemy::Hurtbox()가 반환하는 AABB 사각형
                // 정밀한 도트 게임에서는 나중에 캡슐, 다중 Hurtbox, 약점 박스로 확장할 수 있습니다.
                if (!CircleIntersectsRect(projectile.Position(), projectile.HitRadius(), enemy.Hurtbox()))
                {
                    continue;
                }

                // 피해 적용은 Enemy 내부 규칙으로 처리합니다.
                // 무적, 방어력, 약점 배율, 보스 페이즈 같은 규칙은 ApplyDamage 안으로 들어가는 것이 자연스럽습니다.
                const ProjectileHitPayload& payload = projectile.HitPayload();
                const DamageResult damageResult = enemy.ApplyDamage(payload.damage, currentTimeSeconds);
                if (!damageResult.accepted)
                {
                    continue;
                }

                // 충돌이 실제 피해로 받아들여진 경우에만 HitEvent를 만듭니다.
                // 이 이벤트는 피격 파티클, 사운드, 카메라 피드백, 퀘스트 카운트 같은 후속 시스템의 공통 입력입니다.
                HitEvent event;
                event.sourceId = projectile.SourceId();
                event.targetId = enemy.Definition().id;
                event.hitEffectId = payload.hitEffectId;
                event.position = projectile.Position();
                event.normal = HitNormal(event.position, enemy.Position());
                event.damage = payload.damage;
                event.damageResult = damageResult;
                event.feedback = payload.feedback;
                outHitEvents.push_back(event);

                // 일반 탄은 여기서 사라지고, 관통탄은 남은 관통 수를 줄입니다.
                projectile.ResolveHitLifetime();
            }
        }
    }

    bool CombatHitResolver::CircleIntersectsRect(Vector2 center, float radius, Rectangle rect)
    {
        // 원 중심에서 사각형 안의 가장 가까운 점을 찾습니다.
        // 그 점까지의 거리 제곱이 반지름 제곱보다 작거나 같으면 원과 사각형이 겹칩니다.
        const float nearestX = std::clamp(center.x, rect.x, rect.x + rect.width);
        const float nearestY = std::clamp(center.y, rect.y, rect.y + rect.height);
        const float dx = center.x - nearestX;
        const float dy = center.y - nearestY;
        return dx * dx + dy * dy <= radius * radius;
    }

    Vector2 CombatHitResolver::HitNormal(Vector2 hitPosition, Vector2 targetPosition)
    {
        // targetPosition에서 hitPosition으로 향하는 방향을 사용합니다.
        // 즉 "맞은 표면에서 바깥쪽으로 튀는 방향"에 가까운 값입니다.
        // 위치가 완전히 겹친 경우에는 방향을 정할 수 없으므로 위쪽을 기본값으로 둡니다.
        return math::NormalizeOr(
            math::Subtract(hitPosition, targetPosition),
            { 0.0f, 1.0f });
    }
}

