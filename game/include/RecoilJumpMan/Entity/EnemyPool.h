#pragma once

// EnemyPool.h
// - 맵에 배치된 SpawnPoint 수만큼 Enemy를 미리 만들어둡니다.
// - 매 프레임 전체 적을 돌지 않고, 플레이어 주변 chunk의 적만 활성화 후보로 봅니다.

#include "RecoilJumpMan/Data/EnemyDefinition.h"
#include "RecoilJumpMan/Entity/Enemy.h"

#include <cstddef>
#include <vector>

namespace rjm
{
    class DataRegistry;
    class TileMap;
    class WorldMap;

    class EnemyPool
    {
    public:
        void Initialize(const WorldMap& world, const DataRegistry& data, const EnemyDefinition& fallbackDefinition);
        void Update(
            const WorldMap& world,
            Vector2 playerWorldPosition,
            Vector2 playerWorldVelocity,
            bool playerGrounded,
            double currentTimeSeconds,
            float deltaSeconds);
        void Draw() const;

        // Enemies:
        // - CombatHitResolver가 활성 인덱스를 통해 실제 Enemy 객체에 접근할 수 있게 합니다.
        // - EnemyPool이 소유권을 유지하고, 외부 시스템은 필요한 순간에만 참조합니다.
        std::vector<Enemy>& Enemies();
        const std::vector<Enemy>& Enemies() const;

        // ActiveIndices:
        // - 현재 플레이어 주변에서 업데이트/그리기 대상인 적 인덱스 목록입니다.
        // - 충돌 판정도 이 목록만 순회해 전체 맵의 모든 적을 검사하지 않게 합니다.
        const std::vector<std::size_t>& ActiveIndices() const;

        std::size_t ActiveCount() const;
        std::size_t Capacity() const;

    private:
        void ResolveCrowdSeparation(const TileMap& tileMap);

        std::vector<Enemy> enemies_;
        std::vector<std::size_t> activeIndices_;
        std::vector<std::size_t> candidateSpawnIndices_;
        float maxActivationQueryRadius_ = 0.0f;
    };
}

