#pragma once

// CameraTuning.h
// - 카메라 추적감, 줌, 반동 예측 시야를 조절하는 값들을 모아둔 구조체입니다.
// - 데드존 방식은 반동 이동과 잘 맞지 않으므로 제거했습니다.
// - x축은 속도 예측을 유지하고, y축은 일정 속도 제한으로 따라가 멀미를 줄입니다.

#include <raylib.h>

namespace rjm
{
    struct CameraTuning
    {
        // 기본 줌 배율입니다. 1.0f는 원래 크기입니다.
        float zoom = 0.8f;

        // 마우스 휠 한 칸당 줌이 곱해지는 비율입니다.
        // 1.12면 휠 한 칸에 약 12% 확대/축소됩니다.
        float zoomStep = 1.12f;

        // 최소 줌입니다. 너무 축소해서 맵 밖이 과하게 보이는 것을 막습니다.
        float minZoom = 0.65f;

        // 최대 줌입니다. 너무 확대해서 조작이 답답해지는 것을 막습니다.
        float maxZoom = 1.8f;

        // 플레이어가 화면의 어느 위치에 놓이길 원하는지 나타냅니다.
        // x=0.5는 가로 중앙, y=0.58은 세로 중앙보다 살짝 아래입니다.
        // 반동 점프 게임에서는 위쪽 시야가 조금 더 필요해서 y를 0.5보다 크게 둡니다.
        Vector2 offsetRatio = { 0.5f, 0.58f };

        // 카메라 x축이 최종 목표점을 따라가는 속도입니다.
        // 높을수록 즉각적이고, 낮을수록 묵직합니다.
        float horizontalFollowSharpness = 8.5f;

        // 카메라 y축이 평상시에 따라가는 초당 최대 속도입니다.
        // y축은 반동 물리처럼 급가속/급감속하지 않고, 이 속도 안에서 일정하게 이동합니다.
        float verticalFollowSpeed = 520.0f;

        // 플레이어와 카메라 y축 거리가 크게 벌어졌을 때 사용하는 빠른 따라잡기 속도입니다.
        float verticalCatchUpSpeed = 920.0f;

        // y축 거리가 이 값보다 커지면 catch-up 속도로 전환합니다.
        float verticalCatchUpDistance = 240.0f;

        // 플레이어 속도를 몇 초치 미리 볼지 정합니다.
        // 반동으로 빠르게 날아가는 방향을 조금 먼저 보여주는 값입니다.
        float velocityLookAheadSeconds = 0.3f;

        // 속도 예측 오프셋의 좌우 최대 거리입니다.
        float maxLookAheadX = 0.0f;

        // 속도 예측 오프셋의 상하 최대 거리입니다.
        // 기본적으로 y축 속도 예측은 꺼두는 편이 반동 점프 멀미를 줄입니다.
        float maxLookAheadY = 0.0f;

        // y축 속도 예측 배율입니다.
        // 0이면 y축 velocity look-ahead가 완전히 꺼집니다.
        float verticalVelocityLookAheadScale = 0.0f;

        // 속도 예측 오프셋 자체가 바뀌는 속도입니다.
        // 이 값이 낮으면 반동 방향이 바뀌어도 카메라가 덜 휘청입니다.
        float velocityLookAheadSharpness = 9.0f;

        // 조준 방향의 반대쪽, 즉 반동으로 날아갈 방향에 주는 카메라 bias 거리입니다.
        // 너무 크면 카메라가 마우스에 끌려다니므로 작게 유지합니다.
        float recoilAimBiasDistance = 30.0f;

        // 반동 조준 bias 중 y축에 적용되는 비율입니다.
        // 위아래 조준은 자주 바뀌므로 x축보다 약하게 반영합니다.
        float recoilAimBiasYScale = 0.25f;

        // 마우스가 플레이어로부터 이 거리만큼 떨어졌을 때 aim bias가 100% 적용됩니다.
        // 가까운 곳을 조준할 때 카메라가 불필요하게 흔들리는 것을 막습니다.
        float recoilAimBiasFullDistance = 420.0f;

        // 조준 방향 bias 자체가 바뀌는 속도입니다.
        float recoilAimBiasSharpness = 6.0f;
    };
}

