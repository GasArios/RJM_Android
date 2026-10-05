#pragma once

// RespawnPoint.h
// - HP가 0이 되어 임시 육체가 붕괴했을 때 재구성될 위치입니다.
// - SafePoint가 낙사/끼임 복구용이라면, RespawnPoint는 마을, 캠프, 필드 입구 같은 부활 앵커입니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <raylib.h>

namespace rjm
{
    enum class RespawnPointKind
    {
        Town,
        Camp,
        FieldEntrance,
        BossGate,
        Debug
    };

    struct RespawnPoint
    {
        DefinitionId id;
        Vector2 position = { 0.0f, 0.0f };
        RespawnPointKind kind = RespawnPointKind::FieldEntrance;
        int priority = 0;
    };
}

