#pragma once

// LevelDefinition.h
// - 맵/지역 하나의 기본 데이터를 담습니다.
// - 기획서의 T0~T5 이동 난이도 등급을 코드로 표현합니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <string>

namespace rjm
{
    // TraversalTier:
    // - 플레이어가 해당 구간을 통과하기 위해 필요한 이동 능력 난이도입니다.
    enum class TraversalTier
    {
        T0, // 기본 리볼버로 통과 가능해야 하는 필수 초반 경로입니다.
        T1, // 기본 총으로 가능하지만 조금 어려운 경로입니다.
        T2, // 샷건이나 고반동 총이 필요한 경로입니다.
        T3, // 고화력과 장탄수 조합이 필요한 경로입니다.
        T4, // 공중 재장전이나 특수 스킬이 필요한 경로입니다.
        T5  // 개발자가 의도한 고난도 기행 루트입니다.
    };

    struct LevelDefinition
    {
        // 레벨 데이터의 고유 id입니다.
        DefinitionId id;

        // 화면에 표시할 지역 이름입니다.
        std::string displayName;

        // 실제 맵 파일 경로입니다.
        std::string mapPath;

        // 이 레벨에 진입하거나 주요 경로를 통과하기 위한 최소 이동 난이도입니다.
        TraversalTier minimumTraversalTier = TraversalTier::T0;
    };
}

