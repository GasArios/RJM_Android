#pragma once

// GameCamera.h
// - 메트로베니아 월드를 보여주는 게임 카메라입니다.
// - 데드존 대신 반동 이동에 맞춘 예측 추적을 사용합니다.
// - 플레이어 속도 방향과 반전된 조준 방향을 참고해 다음 이동 시야를 열어줍니다.

#include "RecoilJumpMan/Camera/CameraTuning.h"

#include <raylib.h>

namespace rjm
{
    class GameCamera
    {
    public:
        // 생성자:
        // - Raylib Camera2D 기본값과 튜닝값을 설정합니다.
        GameCamera();

        // SetWorldBounds:
        // - 카메라가 보여줄 수 있는 월드 경계입니다.
        // - 이 Rectangle은 게임 내부 물리 좌표 기준입니다.
        // - x/y는 월드의 왼쪽 아래, width/height는 크기입니다.
        void SetWorldBounds(Rectangle worldBounds);

        // SnapTo:
        // - 보간 없이 즉시 특정 월드 위치로 카메라 초점을 이동합니다.
        // - 씬 시작, 순간이동, 방 전환 등에 사용합니다.
        void SnapTo(Vector2 worldPosition);

        // SetScreenShakeOffset:
        // - 카메라 계산 자체가 아니라 최종 렌더링에만 얹을 화면 흔들림 오프셋입니다.
        // - 조준 좌표 변환이 화면 흔들림에 같이 흔들리지 않도록 Begin에서만 적용합니다.
        void SetScreenShakeOffset(Vector2 screenOffset);

        // Update:
        // - 플레이어 위치/속도, 마우스 월드 좌표, 마우스 휠 입력을 받아 카메라를 갱신합니다.
        // - mouseWorldPosition은 반전된 조준 방향 bias를 계산하는 데 사용합니다.
        void Update(
            Vector2 playerWorldPosition,
            Vector2 playerWorldVelocity,
            Vector2 mouseWorldPosition,
            float mouseWheelMove,
            float deltaSeconds);

        // Begin:
        // - 이 호출 이후 그리는 것은 카메라 영향을 받는 월드 오브젝트가 됩니다.
        void Begin() const;

        // End:
        // - 카메라 월드 그리기 모드를 종료합니다.
        // - HUD 같은 화면 고정 UI는 End 이후에 그려야 합니다.
        void End() const;

        // ScreenToWorld:
        // - 마우스 화면 좌표를 게임 내부 월드 좌표로 변환합니다.
        Vector2 ScreenToWorld(Vector2 screenPosition) const;

        // WorldToScreen:
        // - 게임 내부 월드 좌표를 실제 화면 좌표로 변환합니다.
        Vector2 WorldToScreen(Vector2 worldPosition) const;

        // Camera2D:
        // - Raylib Camera2D를 읽기 전용으로 반환합니다.
        const Camera2D& RawCamera() const;

        // Zoom:
        // - 현재 줌 배율을 반환합니다.
        float Zoom() const;

    private:
        // ApplyZoomInput:
        // - 마우스 휠 입력으로 줌 배율을 갱신합니다.
        void ApplyZoomInput(float mouseWheelMove);

        // BuildDesiredTarget:
        // - 플레이어 위치, 속도 예측, 반전된 조준 방향 bias를 합쳐 목표 카메라 중심을 만듭니다.
        Vector2 BuildDesiredTarget(
            Vector2 playerWorldPosition,
            Vector2 playerWorldVelocity,
            Vector2 mouseWorldPosition,
            float deltaSeconds);

        // ClampTargetToBounds:
        // - 현재 줌/오프셋 기준으로 카메라가 맵 밖을 보여주지 않도록 targetWorld를 제한합니다.
        Vector2 ClampTargetToBounds(Vector2 targetWorld) const;

        // UpdateRaylibCamera:
        // - 내부 월드 좌표 targetWorld_를 Raylib Camera2D target으로 반영합니다.
        void UpdateRaylibCamera();

        // SmoothStepTo:
        // - FPS 차이에 덜 흔들리는 지수 보간으로 current를 target에 가까이 보냅니다.
        static float SmoothStepTo(float current, float target, float sharpness, float deltaSeconds);

        // MoveTowards:
        // - current를 target 쪽으로 최대 maxDelta만큼만 이동시킵니다.
        // - y축 카메라가 급가속하지 않고 일정 속도로 따라가게 할 때 사용합니다.
        static float MoveTowards(float current, float target, float maxDelta);

        // SmoothVectorTo:
        // - Vector2 두 축을 같은 sharpness로 부드럽게 보간합니다.
        static Vector2 SmoothVectorTo(Vector2 current, Vector2 target, float sharpness, float deltaSeconds);

        // Raylib에 실제로 넘기는 카메라입니다.
        Camera2D camera_ = {};

        // 최종 렌더링에만 더하는 화면 픽셀 단위 흔들림입니다.
        Vector2 screenShakeOffset_ = { 0.0f, 0.0f };

        // 카메라가 현재 바라보는 중심점입니다.
        // 게임 내부 물리 좌표 기준입니다.
        Vector2 targetWorld_ = { 0.0f, 0.0f };

        // 부드럽게 보간된 속도 기반 예측 오프셋입니다.
        Vector2 smoothedVelocityLookAhead_ = { 0.0f, 0.0f };

        // 부드럽게 보간된 반동 방향 조준 bias입니다.
        Vector2 smoothedRecoilAimBias_ = { 0.0f, 0.0f };

        // 카메라가 벗어나면 안 되는 월드 경계입니다.
        Rectangle worldBounds_ = { 0.0f, 0.0f, 1280.0f, 720.0f };

        // 카메라 튜닝값 묶음입니다.
        CameraTuning tuning_;
    };
}

