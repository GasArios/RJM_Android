#pragma once

// SpawnPoint.h
// - 적을 실제로 매번 생성/삭제하는 대신, 맵에 배치된 적의 원래 위치와 리스폰 규칙을 담습니다.
// - EnemyPool은 이 SpawnPoint 목록을 기반으로 적 객체를 미리 만들어둡니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <raylib.h>

namespace rjm
{
    struct SpawnPoint
    {
        DefinitionId id;
        DefinitionId enemyDefinitionId;

        // 적이 리스폰될 중심 월드 좌표입니다.
        // Entity::Position() 기본 규칙과 맞춰 중심 좌표로 저장합니다.
        Vector2 position = { 0.0f, 0.0f };

        float activationRadius = 1400.0f;
        float deactivationRadius = 1800.0f;
        float respawnSeconds = 8.0f;
    };
}

