#pragma once

// Area.h
// - Area는 "로딩 단위"가 아니라 "게임플레이 논리 단위"입니다.
// - 예: 숲, 광산, 마을, 탑 같은 지역 이름/난이도/카메라 경계/음악 등을 붙일 수 있습니다.

#include "RecoilJumpMan/Data/DefinitionId.h"
#include "RecoilJumpMan/Data/LevelDefinition.h"

#include <raylib.h>

#include <string>

namespace rjm
{
    struct Area
    {
        DefinitionId id;
        std::string displayName;
        Rectangle bounds = { 0.0f, 0.0f, 0.0f, 0.0f };
        Rectangle cameraBounds = { 0.0f, 0.0f, 0.0f, 0.0f };
        TraversalTier traversalTier = TraversalTier::T0;

        bool Contains(Vector2 worldPosition) const;
    };
}

