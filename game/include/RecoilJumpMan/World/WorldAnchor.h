#pragma once

// WorldAnchor.h
// - 전환, 이벤트, 스폰 후 배치처럼 "id로 찾아가는 월드 좌표"를 표현합니다.
// - 적 SpawnPoint와 분리해 문/포탈의 도착 지점이 적 스폰 데이터와 섞이지 않게 합니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <raylib.h>

namespace rjm
{
    struct WorldAnchor
    {
        DefinitionId id;
        Vector2 position = { 0.0f, 0.0f };
    };
}

