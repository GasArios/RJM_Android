#pragma once

// SafePoint.h
// - 낙하/버그 이탈 복구에 사용할 안전 위치입니다.
// - LDtk의 SafePoint, PlayerStart, Checkpoint 같은 엔티티가 이 런타임 타입으로 변환될 수 있습니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <raylib.h>

namespace rjm
{
    // SafePoint:
    // - 플레이어가 맵 밖으로 떨어지거나 충돌 버그로 이탈했을 때 돌아갈 수 있는 위치입니다.
    // - 위치는 Entity 규칙과 맞춰 중심 월드 좌표로 저장합니다.
    // - LDtk에서는 SafePoint 엔티티로 찍고, 로더가 이 구조체로 변환합니다.
    struct SafePoint
    {
        // 안전 지점의 고유 id입니다.
        // 디버그 표시, 저장 데이터, 특정 구역 복구 규칙에 사용할 수 있습니다.
        DefinitionId id;

        // 복구할 중심 월드 좌표입니다.
        // 플레이어를 이 위치로 옮긴 뒤 속도를 0으로 만들어 낙하/관통 상태를 끊습니다.
        Vector2 position = { 0.0f, 0.0f };

        // 여러 SafePoint가 있을 때 fallback 후보를 고르는 우선순위입니다.
        // 값이 높을수록 우선합니다.
        int priority = 0;
    };
}

