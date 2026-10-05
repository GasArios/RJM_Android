// WorldMap.cpp
// - 거대한 심리스 월드 데이터와 chunk/spawn 인덱스 구현부입니다.

#include "RecoilJumpMan/World/WorldMap.h"

#include "RecoilJumpMan/Physics/CoordinateSpace.h"
#include "RecoilJumpMan/World/LdtkWorldLoader.h"

#include <algorithm>
#include <cmath>
#include <iostream>
#include <string>
#include <utility>

namespace
{
    Rectangle WorldRectToRender(Rectangle worldRect)
    {
        const Vector2 topLeft = rjm::CoordinateSpace::WorldToRender(
            { worldRect.x, worldRect.y + worldRect.height });
        return { topLeft.x, topLeft.y, worldRect.width, worldRect.height };
    }
}

namespace rjm
{
    WorldMap::WorldMap()
    {
        // 기본 맵 파일을 먼저 시도합니다.
        // 파일이 아직 없거나 로딩에 실패하면, 개발을 계속할 수 있도록 코드 기반 디버그 월드를 만듭니다.
        if (!LoadFromLdtkFile("assets/maps/starter_field.ldtk"))
        {
            BuildDebugWorld();
        }
    }

    void WorldMap::BuildDebugWorld()
    {
        tileMap_.Resize(120, 32, 32);
        worldBounds_ = { 0.0f, 0.0f, 3840.0f, 1024.0f };
        floorWorldY_ = 192.0f;
        chunkSizeTiles_ = 16;

        areas_.clear();
        chunks_.clear();
        transitions_.clear();
        anchors_.clear();
        spawnPoints_.clear();
        safePoints_.clear();
        respawnPoints_.clear();

        // 디버그 월드도 LDtk 월드와 같은 시작 위치 규칙을 따릅니다.
        // position은 플레이어 중심 좌표이므로 바닥 높이에 반 칸을 더합니다.
        playerStartPosition_ = Vector2{ 220.0f, floorWorldY_ + 16.0f };

        BuildChunks();

        // 맨 아래 6줄은 기본 바닥입니다.
        // 기존 groundY 평면 대신 실제 solid 타일을 깔아 TileMap 충돌 경로를 테스트합니다.
        for (int x = 0; x < tileMap_.Width(); ++x)
        {
            for (int y = 0; y < 6; ++y)
            {
                tileMap_.SetCollisionFlags(x, y, TileCollisionSolid);
            }
        }

        // 타일 효과 시스템 확인용 샘플 지형입니다.
        // y=6은 기본 바닥 바로 위라서 비고체 볼륨은 플레이어와 겹치고,
        // solid 표면 효과는 한 칸 높은 발판으로 테스트할 수 있습니다.
        for (int x = 32; x < 35; ++x)
        {
            tileMap_.SetEffectFlags(x, 6, TileEffectHazard, "debug_spikes");
        }

        for (int x = 36; x < 41; ++x)
        {
            tileMap_.SetEffectFlags(x, 6, TileEffectLava, "debug_lava");
        }

        for (int x = 53; x < 59; ++x)
        {
            tileMap_.SetEffectFlags(x, 6, TileEffectWater, "debug_water");
            tileMap_.SetEffectFlags(x, 7, TileEffectWater, "debug_water");
        }

        for (int x = 66; x < 73; ++x)
        {
            tileMap_.SetCollisionFlags(x, 6, TileCollisionSolid);
            tileMap_.SetEffectFlags(x, 6, TileEffectIce, "debug_ice");
        }

        for (int x = 75; x < 80; ++x)
        {
            tileMap_.SetCollisionFlags(x, 6, TileCollisionSolid);
            tileMap_.SetEffectFlags(x, 6, TileEffectSticky, "debug_sticky");
        }

        for (int x = 82; x < 88; ++x)
        {
            tileMap_.SetCollisionFlags(x, 6, TileCollisionSolid);
            tileMap_.SetEffectFlags(x, 6, TileEffectConveyorRight, "debug_conveyor_right");
        }

        // 반동 점프와 착지 보정 확인용 중간 발판들입니다.
        for (int x = 18; x < 30; ++x)
        {
            tileMap_.SetCollisionFlags(x, 10, TileCollisionSolid);
        }

        for (int x = 43; x < 51; ++x)
        {
            tileMap_.SetCollisionFlags(x, 14, TileCollisionSolid);
        }

        // 수직 벽 충돌과 벽 끼임 복구 확인용 기둥입니다.
        for (int y = 6; y < 18; ++y)
        {
            tileMap_.SetCollisionFlags(62, y, TileCollisionSolid);
        }

        // 월드 양끝에는 solid 벽을 세워 테스트 중 맵 밖으로 쉽게 이탈하지 않게 합니다.
        for (int y = 6; y < 24; ++y)
        {
            tileMap_.SetCollisionFlags(0, y, TileCollisionSolid);
            tileMap_.SetCollisionFlags(tileMap_.Width() - 1, y, TileCollisionSolid);
        }

        areas_.push_back(Area{ "starter_field", "Starter Field", { 0.0f, 0.0f, 1280.0f, 1024.0f }, { 0.0f, 0.0f, 1280.0f, 1024.0f }, TraversalTier::T0 });
        areas_.push_back(Area{ "old_road", "Old Road", { 1280.0f, 0.0f, 1280.0f, 1024.0f }, { 1280.0f, 0.0f, 1280.0f, 1024.0f }, TraversalTier::T1 });
        areas_.push_back(Area{ "watchtower_edge", "Watchtower Edge", { 2560.0f, 0.0f, 1280.0f, 1024.0f }, { 2560.0f, 0.0f, 1280.0f, 1024.0f }, TraversalTier::T2 });

        anchors_.push_back(WorldAnchor{ "hut_entry", { 520.0f, floorWorldY_ + 16.0f } });
        anchors_.push_back(WorldAnchor{ "west_entry", { 280.0f, floorWorldY_ + 16.0f } });

        transitions_.push_back(TransitionTrigger{ "hut_door", { 620.0f, floorWorldY_, 72.0f, 120.0f }, TransitionKind::Interact, "", "hut_entry" });
        transitions_.push_back(TransitionTrigger{ "east_exit", { worldBounds_.x + worldBounds_.width - 24.0f, floorWorldY_, 24.0f, 260.0f }, TransitionKind::Touch, "", "west_entry" });

        // lastSafePosition이 아직 없을 때 사용할 정적 fallback 지점입니다.
        // LDtk 맵의 SafePoint와 같은 의미로 유지합니다.
        safePoints_.push_back(SafePoint{ "debug_safe_start", { 220.0f, floorWorldY_ + 16.0f }, 0 });

        // HP 0 사망 후에는 마지막 발판이 아니라 필드 시작 앵커에서 임시 육체를 재구성합니다.
        respawnPoints_.push_back(RespawnPoint{
            "debug_respawn_anchor",
            { 220.0f, floorWorldY_ + 16.0f },
            RespawnPointKind::Debug,
            0
        });

        for (int i = 0; i < 12; ++i)
        {
            const float x = 760.0f + static_cast<float>(i) * 230.0f;

            // 디버그 적은 현재 28x28 사각형으로 그립니다.
            // SpawnPoint::position은 중심 좌표이므로, 바닥에 서 있게 하려면 floorWorldY_에 반 크기를 더합니다.
            constexpr float debugEnemyHalfSize = 14.0f;
            AddSpawnPoint(SpawnPoint{
                "debug_enemy_" + std::to_string(i),
                i % 4 == 3 ? "debug_hound" : "debug_slime",
                { x, floorWorldY_ + debugEnemyHalfSize },
                1400.0f,
                1800.0f,
                8.0f
            });
        }
    }

    bool WorldMap::LoadFromLdtkFile(const std::string& path)
    {
        // 로더는 LDtk JSON 구조를 LdtkLoadedWorld로만 변환합니다.
        // WorldMap은 그 결과를 자기 런타임 컨테이너로 옮긴 뒤 chunk 인덱스를 다시 만듭니다.
        LdtkLoadedWorld loadedWorld;
        std::string error;
        const LdtkWorldLoader loader;
        if (!loader.Load(path, loadedWorld, &error))
        {
            if (!error.empty())
            {
                std::cout << "LDtk load skipped: " << error << '\n';
            }
            return false;
        }

        tileMap_ = std::move(loadedWorld.tileMap);
        worldBounds_ = loadedWorld.worldBounds;
        floorWorldY_ = loadedWorld.floorWorldY;
        areas_ = std::move(loadedWorld.areas);
        transitions_ = std::move(loadedWorld.transitions);
        anchors_ = std::move(loadedWorld.anchors);
        spawnPoints_ = std::move(loadedWorld.spawnPoints);
        safePoints_ = std::move(loadedWorld.safePoints);
        respawnPoints_ = std::move(loadedWorld.respawnPoints);
        playerStartPosition_ = loadedWorld.playerStartPosition;
        chunkSizeTiles_ = 16;

        // SpawnPoint는 chunk별 인덱스를 따로 들고 있으므로, 로드 후 반드시 다시 연결합니다.
        RebuildChunks();
        return true;
    }

    void WorldMap::Draw() const
    {
        tileMap_.DrawDebugCollision();
        tileMap_.DrawDebugGrid();

        DrawLineV(
            CoordinateSpace::WorldToRender({ worldBounds_.x, floorWorldY_ }),
            CoordinateSpace::WorldToRender({ worldBounds_.x + worldBounds_.width, floorWorldY_ }),
            Color{ 98, 120, 145, 255 });

        for (const Area& area : areas_)
        {
            DrawRectangleLinesEx(WorldRectToRender(area.bounds), 2.0f, Color{ 70, 82, 100, 180 });
        }

        for (const TransitionTrigger& trigger : transitions_)
        {
            const Color color = trigger.kind == TransitionKind::Interact
                ? Color{ 90, 180, 255, 180 }
                : Color{ 255, 190, 90, 180 };
            DrawRectangleLinesEx(WorldRectToRender(trigger.bounds), 2.0f, color);
        }

        for (const WorldAnchor& anchor : anchors_)
        {
            const Vector2 renderPosition = CoordinateSpace::WorldToRender(anchor.position);
            DrawCircleLines(
                static_cast<int>(renderPosition.x),
                static_cast<int>(renderPosition.y),
                8.0f,
                Color{ 210, 160, 255, 185 });
            DrawCircleV(renderPosition, 2.5f, Color{ 210, 160, 255, 170 });
        }

        for (const RespawnPoint& respawnPoint : respawnPoints_)
        {
            const Vector2 renderPosition = CoordinateSpace::WorldToRender(respawnPoint.position);
            DrawCircleLines(
                static_cast<int>(renderPosition.x),
                static_cast<int>(renderPosition.y),
                12.0f,
                Color{ 120, 230, 210, 210 });
            DrawCircleV(renderPosition, 4.0f, Color{ 120, 230, 210, 190 });
        }
    }

    Rectangle WorldMap::WorldBounds() const
    {
        return worldBounds_;
    }

    float WorldMap::FloorWorldY() const
    {
        return floorWorldY_;
    }

    const TileMap& WorldMap::Map() const
    {
        return tileMap_;
    }

    TileMap& WorldMap::Map()
    {
        return tileMap_;
    }

    const std::vector<Area>& WorldMap::Areas() const
    {
        return areas_;
    }

    const std::vector<TileChunk>& WorldMap::Chunks() const
    {
        return chunks_;
    }

    const std::vector<TransitionTrigger>& WorldMap::Transitions() const
    {
        return transitions_;
    }

    const std::vector<WorldAnchor>& WorldMap::Anchors() const
    {
        return anchors_;
    }

    const std::vector<SpawnPoint>& WorldMap::SpawnPoints() const
    {
        return spawnPoints_;
    }

    const std::vector<SafePoint>& WorldMap::SafePoints() const
    {
        return safePoints_;
    }

    const std::vector<RespawnPoint>& WorldMap::RespawnPoints() const
    {
        return respawnPoints_;
    }

    std::optional<Vector2> WorldMap::PlayerStartPosition() const
    {
        return playerStartPosition_;
    }

    std::optional<Vector2> WorldMap::FirstSafePointPosition() const
    {
        if (safePoints_.empty())
        {
            return std::nullopt;
        }

        // 현재는 "가장 높은 priority" 하나를 전역 fallback으로 사용합니다.
        // 나중에 지역별 복구가 필요해지면 플레이어 위치와 Area를 같이 보고 고를 수 있습니다.
        const SafePoint* best = &safePoints_.front();
        for (const SafePoint& safePoint : safePoints_)
        {
            if (safePoint.priority > best->priority)
            {
                best = &safePoint;
            }
        }

        return best->position;
    }

    std::optional<Vector2> WorldMap::FirstRespawnPointPosition() const
    {
        if (respawnPoints_.empty())
        {
            return std::nullopt;
        }

        const RespawnPoint* best = &respawnPoints_.front();
        for (const RespawnPoint& respawnPoint : respawnPoints_)
        {
            if (respawnPoint.priority > best->priority)
            {
                best = &respawnPoint;
            }
        }

        return best->position;
    }

    const TransitionTrigger* WorldMap::FindTransitionAt(Vector2 worldPosition, bool interactPressed) const
    {
        for (const TransitionTrigger& transition : transitions_)
        {
            if (!transition.Contains(worldPosition))
            {
                continue;
            }

            if (transition.kind == TransitionKind::Touch || interactPressed)
            {
                return &transition;
            }
        }

        return nullptr;
    }

    const WorldAnchor* WorldMap::FindAnchor(const DefinitionId& id) const
    {
        for (const WorldAnchor& anchor : anchors_)
        {
            if (anchor.id == id)
            {
                return &anchor;
            }
        }

        return nullptr;
    }

    void WorldMap::CollectSpawnPointsNear(Vector2 worldPosition, float radius, std::vector<std::size_t>& outIndices) const
    {
        outIndices.clear();

        const Rectangle query = {
            worldPosition.x - radius,
            worldPosition.y - radius,
            radius * 2.0f,
            radius * 2.0f
        };

        for (const TileChunk& chunk : chunks_)
        {
            if (!chunk.Intersects(query))
            {
                continue;
            }

            for (std::size_t spawnIndex : chunk.spawnPointIndices)
            {
                outIndices.push_back(spawnIndex);
            }
        }
    }

    void WorldMap::BuildChunks()
    {
        chunks_.clear();

        const int chunkWorldSize = chunkSizeTiles_ * tileMap_.TileSize();
        const int chunkColumns = static_cast<int>(std::ceil(worldBounds_.width / static_cast<float>(chunkWorldSize)));
        const int chunkRows = static_cast<int>(std::ceil(worldBounds_.height / static_cast<float>(chunkWorldSize)));

        chunks_.reserve(chunkColumns * chunkRows);

        for (int y = 0; y < chunkRows; ++y)
        {
            for (int x = 0; x < chunkColumns; ++x)
            {
                TileChunk chunk;
                chunk.coord = { x, y };
                chunk.bounds = {
                    worldBounds_.x + static_cast<float>(x * chunkWorldSize),
                    worldBounds_.y + static_cast<float>(y * chunkWorldSize),
                    static_cast<float>(chunkWorldSize),
                    static_cast<float>(chunkWorldSize)
                };
                chunks_.push_back(std::move(chunk));
            }
        }
    }

    void WorldMap::RebuildChunks()
    {
        BuildChunks();

        for (std::size_t spawnIndex = 0; spawnIndex < spawnPoints_.size(); ++spawnIndex)
        {
            const int chunkIndex = ChunkIndexForPosition(spawnPoints_[spawnIndex].position);
            if (chunkIndex >= 0)
            {
                chunks_[static_cast<std::size_t>(chunkIndex)].spawnPointIndices.push_back(spawnIndex);
            }
        }
    }

    void WorldMap::AddSpawnPoint(const SpawnPoint& spawnPoint)
    {
        const std::size_t spawnIndex = spawnPoints_.size();
        spawnPoints_.push_back(spawnPoint);

        const int chunkIndex = ChunkIndexForPosition(spawnPoint.position);
        if (chunkIndex >= 0)
        {
            chunks_[static_cast<std::size_t>(chunkIndex)].spawnPointIndices.push_back(spawnIndex);
        }
    }

    int WorldMap::ChunkIndexForPosition(Vector2 worldPosition) const
    {
        for (std::size_t i = 0; i < chunks_.size(); ++i)
        {
            if (chunks_[i].Contains(worldPosition))
            {
                return static_cast<int>(i);
            }
        }

        return -1;
    }
}
