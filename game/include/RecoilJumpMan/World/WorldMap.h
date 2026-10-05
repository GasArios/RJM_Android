#pragma once

// WorldMap.h
// - 하나의 큰 2D 메트로베니아 맵 전체를 표현합니다.
// - 맵 데이터는 통째로 메모리에 올라오지만, 내부적으로는 chunk 인덱스를 사용해 주변 데이터만 빠르게 찾습니다.

#include "RecoilJumpMan/World/Area.h"
#include "RecoilJumpMan/World/RespawnPoint.h"
#include "RecoilJumpMan/World/SafePoint.h"
#include "RecoilJumpMan/World/SpawnPoint.h"
#include "RecoilJumpMan/World/TileChunk.h"
#include "RecoilJumpMan/World/TileMap.h"
#include "RecoilJumpMan/World/TransitionTrigger.h"
#include "RecoilJumpMan/World/WorldAnchor.h"

#include <raylib.h>

#include <cstddef>
#include <optional>
#include <string>
#include <vector>

namespace rjm
{
    class WorldMap
    {
    public:
        WorldMap();

        void BuildDebugWorld();
        bool LoadFromLdtkFile(const std::string& path);
        void Draw() const;

        Rectangle WorldBounds() const;
        float FloorWorldY() const;

        const TileMap& Map() const;
        TileMap& Map();

        const std::vector<Area>& Areas() const;
        const std::vector<TileChunk>& Chunks() const;
        const std::vector<TransitionTrigger>& Transitions() const;
        const std::vector<WorldAnchor>& Anchors() const;
        const std::vector<SpawnPoint>& SpawnPoints() const;

        // SafePoints:
        // - LDtk SafePoint 엔티티에서 읽은 복구 후보 목록을 반환합니다.
        const std::vector<SafePoint>& SafePoints() const;

        // RespawnPoints:
        // - HP 0 사망 후 임시 육체를 다시 재구성할 부활 앵커 목록입니다.
        const std::vector<RespawnPoint>& RespawnPoints() const;

        // PlayerStartPosition:
        // - LDtk PlayerStart 엔티티가 있으면 그 중심 월드 좌표를 반환합니다.
        // - 없으면 nullopt를 반환하고, 호출자가 디버그/fallback 위치를 고릅니다.
        std::optional<Vector2> PlayerStartPosition() const;

        // FirstSafePointPosition:
        // - 저장된 SafePoint 중 priority가 가장 높은 위치를 fallback 복구 지점으로 반환합니다.
        std::optional<Vector2> FirstSafePointPosition() const;

        // FirstRespawnPointPosition:
        // - 저장된 RespawnPoint 중 priority가 가장 높은 위치를 기본 부활 지점으로 반환합니다.
        std::optional<Vector2> FirstRespawnPointPosition() const;

        const TransitionTrigger* FindTransitionAt(Vector2 worldPosition, bool interactPressed) const;
        const WorldAnchor* FindAnchor(const DefinitionId& id) const;
        void CollectSpawnPointsNear(Vector2 worldPosition, float radius, std::vector<std::size_t>& outIndices) const;

    private:
        void BuildChunks();
        void RebuildChunks();
        void AddSpawnPoint(const SpawnPoint& spawnPoint);
        int ChunkIndexForPosition(Vector2 worldPosition) const;

        TileMap tileMap_;
        Rectangle worldBounds_ = { 0.0f, 0.0f, 3840.0f, 1024.0f };
        float floorWorldY_ = 180.0f;
        int chunkSizeTiles_ = 16;

        std::vector<Area> areas_;
        std::vector<TileChunk> chunks_;
        std::vector<TransitionTrigger> transitions_;
        std::vector<WorldAnchor> anchors_;
        std::vector<SpawnPoint> spawnPoints_;

        // 맵 이탈 복구용 안전 지점 목록입니다.
        // 플레이 중 갱신되는 lastSafePosition과 달리, 이 값들은 LDtk/디버그 월드가 제공하는 정적 지점입니다.
        std::vector<SafePoint> safePoints_;

        // HP 0 사망 후 돌아갈 부활 앵커 목록입니다.
        std::vector<RespawnPoint> respawnPoints_;

        // LDtk PlayerStart 엔티티에서 읽은 시작 위치입니다.
        // optional인 이유는 맵 파일이 시작점을 제공하지 않을 수도 있기 때문입니다.
        std::optional<Vector2> playerStartPosition_;
    };
}

