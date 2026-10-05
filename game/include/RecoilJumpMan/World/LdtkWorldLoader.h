#pragma once

// LdtkWorldLoader.h
// - LDtk 제작용 데이터를 Recoil Jump Man의 런타임 월드 데이터로 변환합니다.
// - LDtk의 JSON 구조는 이 로더 경계 안에만 가두고,
//   게임플레이 코드는 WorldMap, TileMap, SpawnPoint 같은 내부 타입만 보게 합니다.

#include "RecoilJumpMan/World/Area.h"
#include "RecoilJumpMan/World/RespawnPoint.h"
#include "RecoilJumpMan/World/SafePoint.h"
#include "RecoilJumpMan/World/SpawnPoint.h"
#include "RecoilJumpMan/World/TileMap.h"
#include "RecoilJumpMan/World/TransitionTrigger.h"
#include "RecoilJumpMan/World/WorldAnchor.h"

#include <raylib.h>

#include <optional>
#include <string>
#include <vector>

namespace rjm
{
    // LdtkLoadOptions:
    // - LDtk 파일을 읽을 때 필요한 선택 옵션입니다.
    // - 프로젝트마다 레이어 이름이 달라질 수 있으므로, 이름을 코드 상수로 박지 않고 옵션으로 둡니다.
    struct LdtkLoadOptions
    {
        // 읽을 레벨 identifier입니다.
        // 비어 있으면 LDtk 프로젝트의 첫 번째 레벨을 사용합니다.
        std::string levelIdentifier;

        // 충돌 IntGrid 레이어 이름입니다.
        // 이 레이어의 값이 TileMap의 collisionFlags/effectFlags로 변환됩니다.
        std::string collisionLayerName = "IntGrid_Collision";

        // 엔티티 레이어 이름 접두사입니다.
        // 현재 최소 로더는 __type == Entities를 기준으로 읽지만, 나중에 특정 레이어만 골라 읽을 때 사용할 수 있습니다.
        std::string entityLayerNamePrefix = "Entities";

        // LDtk 레이어에서 grid size를 찾지 못했을 때 사용할 기본 타일 크기입니다.
        int defaultTileSize = 32;
    };

    // LdtkLoadedWorld:
    // - LDtk 로더가 파일에서 읽어낸 중간 결과입니다.
    // - WorldMap은 이 값을 받아 자기 멤버로 옮기고, 이후 게임플레이는 LDtk를 전혀 알 필요가 없습니다.
    struct LdtkLoadedWorld
    {
        // 런타임 타일맵입니다.
        // 충돌 IntGrid와 일부 visual tile id가 여기로 들어옵니다.
        TileMap tileMap;

        // 월드 전체 경계입니다.
        // 카메라 제한, 맵 이탈 복구, chunk 범위 계산에 사용합니다.
        Rectangle worldBounds = { 0.0f, 0.0f, 0.0f, 0.0f };

        // 기존 평면 바닥 기반 코드와의 호환용 기준 높이입니다.
        // 타일 충돌이 중심이 된 뒤에는 주로 디버그/fallback 용도로 남습니다.
        float floorWorldY = 0.0f;

        // 지역 구역입니다.
        // AreaZone 엔티티가 있으면 이 목록으로 변환됩니다.
        std::vector<Area> areas;

        // 문, 방 이동, 맵 경계 이동 같은 전환 트리거입니다.
        std::vector<TransitionTrigger> transitions;

        // 전환 트리거가 도착 지점으로 사용할 앵커입니다.
        std::vector<WorldAnchor> anchors;

        // 적 스폰 포인트입니다.
        std::vector<SpawnPoint> spawnPoints;

        // 맵 이탈 복구용 안전 지점입니다.
        std::vector<SafePoint> safePoints;

        // HP 0 사망 후 임시 육체를 재구성할 부활 앵커입니다.
        std::vector<RespawnPoint> respawnPoints;

        // 플레이어 시작 위치입니다.
        // LDtk PlayerStart 엔티티가 있을 때만 값이 들어갑니다.
        std::optional<Vector2> playerStartPosition;
    };

    // LdtkWorldLoader:
    // - LDtk JSON 파일을 읽고 LdtkLoadedWorld를 채우는 클래스입니다.
    // - 실패 이유는 outError에 짧은 문자열로 돌려줍니다.
    class LdtkWorldLoader
    {
    public:
        // Load:
        // - path 위치의 .ldtk 또는 .ldtkl JSON 파일을 읽습니다.
        // - 성공하면 outWorld를 채우고 true를 반환합니다.
        // - 실패하면 false를 반환하고, outError가 있으면 이유를 적습니다.
        bool Load(
            const std::string& path,
            LdtkLoadedWorld& outWorld,
            std::string* outError = nullptr,
            const LdtkLoadOptions& options = {}) const;
    };
}

