#pragma once

// PlayerEnvironmentModifiers.h
// - 물, 바람, 점액, 컨베이어 같은 지형 효과가 플레이어 물리에 주는 보정값입니다.
// - 타일 효과 판정 코드는 이 구조체만 만들고, 실제 속도 계산은 RecoilMovementController가 담당합니다.

#include <raylib.h>

namespace rjm
{
    struct PlayerEnvironmentModifiers
    {
        // 중력 배율입니다. 0.45라면 물속처럼 중력이 약해집니다.
        float gravityScale = 1.0f;

        // 상승 저항 배율입니다. 1보다 크면 위로 솟는 속도가 더 빨리 죽습니다.
        float upwardDragScale = 1.0f;

        // 정점 근처 중력 배율입니다.
        float apexGravityScale = 1.0f;

        // 공중 수평 저항 배율입니다. 1보다 크면 수평 속도가 더 빨리 죽습니다.
        float airDragScale = 1.0f;

        // 지상 마찰 배율입니다. 1보다 작으면 얼음처럼 미끄럽고, 크면 점액처럼 끈적합니다.
        float groundFrictionScale = 1.0f;

        // 환경이 매 프레임 더하는 가속도입니다. 바람, 물살, 자기장 같은 효과에 사용합니다.
        Vector2 continuousAcceleration = { 0.0f, 0.0f };

        // 바닥이 플레이어를 운반하는 속도입니다. 컨베이어/움직이는 발판 확장 통로입니다.
        Vector2 surfaceVelocity = { 0.0f, 0.0f };

        // 환경별 속도 상한 배율입니다.
        float maxRiseSpeedScale = 1.0f;
        float maxFallSpeedScale = 1.0f;
        float maxHorizontalSpeedScale = 1.0f;

        bool inWater = false;
        bool onReloadSurface = false;
    };
}

