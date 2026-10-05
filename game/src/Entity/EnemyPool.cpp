// EnemyPool.cpp
// - 적 풀의 초기화, 주변 활성화, 리스폰 업데이트 구현부입니다.

#include "RecoilJumpMan/Entity/EnemyPool.h"

#include "RecoilJumpMan/Data/DataRegistry.h"
#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/World/TileMap.h"
#include "RecoilJumpMan/World/WorldMap.h"

#include <algorithm>
#include <cmath>

namespace
{
    // RectanglesOverlap:
    // - y-up 월드 좌표계에서도 AABB 겹침은 x/y 범위의 교집합으로 계산할 수 있습니다.
    // - overlapX/overlapY는 "얼마나 겹쳤는가"를 반환해 군중 분리 강도 계산에 사용합니다.
    // - 단순히 true/false만 반환하지 않는 이유는, 적끼리 살짝 겹친 것은 허용하고
    //   과하게 포개졌을 때만 밀어내기 위해서입니다.
    bool RectanglesOverlap(Rectangle a, Rectangle b, float& overlapX, float& overlapY)
    {
        const float left = std::max(a.x, b.x);
        const float right = std::min(a.x + a.width, b.x + b.width);
        const float bottom = std::max(a.y, b.y);
        const float top = std::min(a.y + a.height, b.y + b.height);

        overlapX = right - left;
        overlapY = top - bottom;
        return overlapX > 0.0f && overlapY > 0.0f;
    }
}

namespace rjm
{
    void EnemyPool::Initialize(const WorldMap& world, const DataRegistry& data, const EnemyDefinition& fallbackDefinition)
    {
        enemies_.clear();
        activeIndices_.clear();
        candidateSpawnIndices_.clear();
        maxActivationQueryRadius_ = 0.0f;

        const auto& spawns = world.SpawnPoints();
        enemies_.resize(spawns.size());

        for (std::size_t i = 0; i < spawns.size(); ++i)
        {
            const EnemyDefinition* definition = data.FindEnemy(spawns[i].enemyDefinitionId);
            enemies_[i].Initialize(spawns[i], definition ? *definition : fallbackDefinition);
            maxActivationQueryRadius_ = std::max(maxActivationQueryRadius_, spawns[i].activationRadius);
            maxActivationQueryRadius_ = std::max(maxActivationQueryRadius_, spawns[i].deactivationRadius);
        }
    }

    void EnemyPool::Update(
        const WorldMap& world,
        Vector2 playerWorldPosition,
        Vector2 playerWorldVelocity,
        bool playerGrounded,
        double currentTimeSeconds,
        float deltaSeconds)
    {
        const EnemyAiContext aiContext{
            playerWorldPosition,
            playerWorldVelocity,
            playerGrounded
        };

        world.CollectSpawnPointsNear(playerWorldPosition, maxActivationQueryRadius_, candidateSpawnIndices_);

        const auto& spawns = world.SpawnPoints();

        for (std::size_t spawnIndex : candidateSpawnIndices_)
        {
            if (spawnIndex >= enemies_.size())
            {
                continue;
            }

            Enemy& enemy = enemies_[spawnIndex];
            const SpawnPoint& spawn = spawns[spawnIndex];
            const float activationDistanceSq = spawn.activationRadius * spawn.activationRadius;

            if (!enemy.IsActive() && math::DistanceSquared(playerWorldPosition, spawn.position) <= activationDistanceSq)
            {
                enemy.SetActive(true);
                activeIndices_.push_back(spawnIndex);
            }
        }

        for (std::size_t i = 0; i < activeIndices_.size();)
        {
            const std::size_t enemyIndex = activeIndices_[i];
            Enemy& enemy = enemies_[enemyIndex];
            const SpawnPoint& spawn = enemy.Spawn();
            const float deactivationDistanceSq = spawn.deactivationRadius * spawn.deactivationRadius;

            if (math::DistanceSquared(playerWorldPosition, spawn.position) > deactivationDistanceSq)
            {
                enemy.SetActive(false);
                activeIndices_[i] = activeIndices_.back();
                activeIndices_.pop_back();
                continue;
            }

            enemy.Update(deltaSeconds, currentTimeSeconds, aiContext, world.Map());
            ++i;
        }

        ResolveCrowdSeparation(world.Map());
    }

    // ResolveCrowdSeparation:
    // - active 적끼리 너무 많이 겹쳤을 때만 x축으로 부드럽게 벌립니다.
    // - 완전 고체 충돌로 만들지 않는 이유:
    //   좁은 통로에서 적이 서로 끼거나, 돌진/점프 적이 다른 적에게 막혀 이상하게 멈출 수 있기 때문입니다.
    // - 따라서 이 함수는 "게임플레이 충돌"이 아니라 "시각적으로 한 덩어리처럼 보이지 않게 하는 후처리"입니다.
    void EnemyPool::ResolveCrowdSeparation(const TileMap& tileMap)
    {
        // 현재는 active 적 수가 많지 않은 프로토타입 단계이므로 모든 쌍을 검사합니다.
        // 나중에 한 화면에 수십~수백 마리가 나오면 타일/그리드 기반 근접 후보만 검사하도록 바꿀 수 있습니다.
        for (std::size_t aActiveIndex = 0; aActiveIndex < activeIndices_.size(); ++aActiveIndex)
        {
            const std::size_t aEnemyIndex = activeIndices_[aActiveIndex];
            if (aEnemyIndex >= enemies_.size())
            {
                continue;
            }

            Enemy& a = enemies_[aEnemyIndex];
            if (!a.IsActive() || !a.IsAlive())
            {
                continue;
            }

            for (std::size_t bActiveIndex = aActiveIndex + 1; bActiveIndex < activeIndices_.size(); ++bActiveIndex)
            {
                const std::size_t bEnemyIndex = activeIndices_[bActiveIndex];
                if (bEnemyIndex >= enemies_.size())
                {
                    continue;
                }

                Enemy& b = enemies_[bEnemyIndex];
                if (!b.IsActive() || !b.IsAlive())
                {
                    continue;
                }

                const EnemyDefinition& aDefinition = a.Definition();
                const EnemyDefinition& bDefinition = b.Definition();
                const bool canPushA = aDefinition.crowdSeparationEnabled;
                const bool canPushB = bDefinition.crowdSeparationEnabled;
                // 둘 다 분리를 받지 않는 적이면 그대로 겹치게 둡니다.
                // 유령, 환영, 소환 이펙트처럼 물리 군중감을 의도하지 않는 적을 위한 예외입니다.
                if (!canPushA && !canPushB)
                {
                    continue;
                }

                float overlapX = 0.0f;
                float overlapY = 0.0f;
                const Rectangle aBox = a.Hurtbox();
                const Rectangle bBox = b.Hurtbox();
                if (!RectanglesOverlap(aBox, bBox, overlapX, overlapY))
                {
                    continue;
                }

                // y축 겹침이 너무 작으면 다른 층이나 다른 발판에 있는 적으로 봅니다.
                // 이 조건이 없으면 위층 적과 아래층 적이 x좌표만 비슷하다는 이유로 서로 밀어낼 수 있습니다.
                const float minimumHeight = std::min(aBox.height, bBox.height);
                const float verticalOverlapRatio = std::max(
                    aDefinition.crowdSeparationVerticalOverlapRatio,
                    bDefinition.crowdSeparationVerticalOverlapRatio);
                if (overlapY < minimumHeight * std::max(0.0f, verticalOverlapRatio))
                {
                    continue;
                }

                // 어느 정도의 가로 겹침은 허용합니다.
                // 몬스터가 무리를 이뤄 몰려오는 느낌은 유지하되, 완전히 한 사각형으로 포개지는 것만 풀기 위함입니다.
                const float allowedOverlapRatio = std::max(
                    0.0f,
                    (aDefinition.crowdSeparationAllowedOverlapRatio + bDefinition.crowdSeparationAllowedOverlapRatio) * 0.5f);
                const float allowedOverlap = std::min(aBox.width, bBox.width) * allowedOverlapRatio;
                const float excessOverlap = overlapX - allowedOverlap;
                if (excessOverlap <= 0.0f)
                {
                    continue;
                }

                // 겹친 만큼을 전부 한 프레임에 풀면 적이 튕겨 나가는 느낌이 납니다.
                // strength와 maxPush로 한 프레임 보정량을 제한해 부드럽게 벌어지게 합니다.
                const float strength = std::max(
                    0.0f,
                    (aDefinition.crowdSeparationStrength + bDefinition.crowdSeparationStrength) * 0.5f);
                const float pairMaxPush = std::max(
                    0.0f,
                    (aDefinition.crowdSeparationMaxPush + bDefinition.crowdSeparationMaxPush) * 0.5f);
                const float totalPush = std::min(excessOverlap * strength, pairMaxPush);
                if (totalPush <= 0.0f)
                {
                    continue;
                }

                // x좌표가 완전히 같을 때는 인덱스 순서로 방향을 결정합니다.
                // 그래야 0 방향이 되어 영원히 같은 위치에 남는 상황을 피할 수 있습니다.
                const float directionFromAToB = b.Position().x > a.Position().x
                    ? 1.0f
                    : (b.Position().x < a.Position().x ? -1.0f : (bEnemyIndex > aEnemyIndex ? 1.0f : -1.0f));
                const float pushA = canPushA && canPushB ? totalPush * 0.5f : (canPushA ? totalPush : 0.0f);
                const float pushB = canPushA && canPushB ? totalPush * 0.5f : (canPushB ? totalPush : 0.0f);

                // 실제 위치 보정은 Enemy 내부 함수에 맡깁니다.
                // Enemy::ApplyCrowdSeparationPush가 TileCollisionResolver를 다시 통과시키므로,
                // 군중 분리 때문에 벽 안으로 밀려 들어가는 일을 막을 수 있습니다.
                if (pushA > 0.0f)
                {
                    a.ApplyCrowdSeparationPush({ -directionFromAToB * pushA, 0.0f }, tileMap);
                }

                if (pushB > 0.0f)
                {
                    b.ApplyCrowdSeparationPush({ directionFromAToB * pushB, 0.0f }, tileMap);
                }
            }
        }
    }

    void EnemyPool::Draw() const
    {
        for (std::size_t enemyIndex : activeIndices_)
        {
            enemies_[enemyIndex].Draw();
        }
    }

    std::vector<Enemy>& EnemyPool::Enemies()
    {
        // CombatHitResolver가 ActiveIndices()로 고른 적을 수정해야 하므로 non-const 참조를 제공합니다.
        // 예: Enemy::ApplyDamage를 호출하면 체력과 사망 상태가 바뀝니다.
        return enemies_;
    }

    const std::vector<Enemy>& EnemyPool::Enemies() const
    {
        return enemies_;
    }

    const std::vector<std::size_t>& EnemyPool::ActiveIndices() const
    {
        // 활성 인덱스는 EnemyPool::Update에서 플레이어 위치를 기준으로 갱신됩니다.
        // 충돌 판정은 이 목록만 보므로 멀리 있는 적까지 검사하지 않습니다.
        return activeIndices_;
    }

    std::size_t EnemyPool::ActiveCount() const
    {
        return activeIndices_.size();
    }

    std::size_t EnemyPool::Capacity() const
    {
        return enemies_.size();
    }

}

