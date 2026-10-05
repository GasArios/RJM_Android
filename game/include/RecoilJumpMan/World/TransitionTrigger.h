#pragma once

// TransitionTrigger.h
// - 건물 입장 포탈, 맵 가장자리 이동, 문 등을 하나의 구조로 표현합니다.
// - "닿으면 이동"과 "상호작용 키를 누르면 이동"을 같은 타입으로 처리합니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <raylib.h>

namespace rjm
{
    enum class TransitionKind
    {
        Touch,
        Interact
    };

    struct TransitionTrigger
    {
        DefinitionId id;
        Rectangle bounds = { 0.0f, 0.0f, 0.0f, 0.0f };
        TransitionKind kind = TransitionKind::Touch;
        DefinitionId targetMapId;
        DefinitionId targetSpawnId;

        bool Contains(Vector2 worldPosition) const;
        bool RequiresInteraction() const;
    };
}

