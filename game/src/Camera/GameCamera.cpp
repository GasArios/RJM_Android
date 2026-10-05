// GameCamera.cpp
// - GameCamera 클래스의 구현부입니다.
// - 데드존을 사용하지 않고, 반동 이동에 맞춘 예측 카메라를 구현합니다.
// - 목표점 = 플레이어 중심 위치 + 속도 예측 + 반전된 조준 방향 bias 입니다.

#include "RecoilJumpMan/Camera/GameCamera.h"

#include "RecoilJumpMan/Core/Config.h"
#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/Physics/CoordinateSpace.h"

#include <algorithm>
#include <cmath>

namespace
{
    // ClampWithCollapsedRange:
    // - 일반적인 clamp는 minimum <= maximum이라는 전제가 있습니다.
    // - 그런데 맵이 화면보다 작거나 줌이 많이 빠진 경우, 카메라가 허용되는 최소/최대 범위가 뒤집힐 수 있습니다.
    // - 그럴 때는 억지로 한쪽 끝에 붙이지 않고, 두 값의 중간을 사용해 카메라를 안정적으로 둡니다.
    float ClampWithCollapsedRange(float value, float minimum, float maximum)
    {
        if (minimum > maximum)
        {
            return (minimum + maximum) * 0.5f;
        }

        return std::clamp(value, minimum, maximum);
    }
}

namespace rjm
{
    GameCamera::GameCamera()
    {
        camera_.rotation = 0.0f;
        camera_.zoom = tuning_.zoom;
        UpdateRaylibCamera();
    }

    void GameCamera::SetWorldBounds(Rectangle worldBounds)
    {
        worldBounds_ = worldBounds;
        targetWorld_ = ClampTargetToBounds(targetWorld_);
        UpdateRaylibCamera();
    }

    void GameCamera::SnapTo(Vector2 worldPosition)
    {
        targetWorld_ = ClampTargetToBounds(worldPosition);
        smoothedVelocityLookAhead_ = { 0.0f, 0.0f };
        smoothedRecoilAimBias_ = { 0.0f, 0.0f };
        UpdateRaylibCamera();
    }

    void GameCamera::SetScreenShakeOffset(Vector2 screenOffset)
    {
        // screenShakeOffset_은 월드 좌표가 아니라 화면 픽셀 좌표입니다.
        // 예: { 5, -2 }라면 이번 프레임에 렌더 카메라를 오른쪽 5픽셀, 위쪽 2픽셀 정도 흔드는 뜻입니다.
        // 실제 camera_.target은 건드리지 않으므로, 조준 좌표와 카메라 추적 계산은 안정적으로 유지됩니다.
        screenShakeOffset_ = screenOffset;
    }

    void GameCamera::Update(
        Vector2 playerWorldPosition,
        Vector2 playerWorldVelocity,
        Vector2 mouseWorldPosition,
        float mouseWheelMove,
        float deltaSeconds)
    {
        ApplyZoomInput(mouseWheelMove);

        const Vector2 desiredTarget = BuildDesiredTarget(
            playerWorldPosition,
            playerWorldVelocity,
            mouseWorldPosition,
            deltaSeconds);

        // x축은 기존처럼 부드러운 예측 추적을 사용합니다.
        targetWorld_.x = SmoothStepTo(targetWorld_.x, desiredTarget.x, tuning_.horizontalFollowSharpness, deltaSeconds);

        // y축은 멀미를 줄이기 위해 초당 이동 속도에 상한을 둡니다.
        // 플레이어가 급상승/급하강해도 카메라 y속도는 1차함수적으로 제한됩니다.
        const float verticalDistance = std::fabs(desiredTarget.y - targetWorld_.y);
        const float verticalSpeed = verticalDistance > tuning_.verticalCatchUpDistance
            ? tuning_.verticalCatchUpSpeed
            : tuning_.verticalFollowSpeed;
        targetWorld_.y = MoveTowards(targetWorld_.y, desiredTarget.y, verticalSpeed * deltaSeconds);

        targetWorld_ = ClampTargetToBounds(targetWorld_);

        UpdateRaylibCamera();
    }

    void GameCamera::Begin() const
    {
        // camera_를 직접 바꾸지 않고 복사본을 만들어 흔들림 offset만 더합니다.
        // 이 함수는 그리기 직전에만 호출되므로, 흔들림은 "보이는 화면"에만 적용됩니다.
        // ScreenToWorld 같은 입력/조준 계산은 원본 camera_를 쓰기 때문에 화면 흔들림의 영향을 덜 받습니다.
        Camera2D renderCamera = camera_;
        renderCamera.offset.x += screenShakeOffset_.x;
        renderCamera.offset.y += screenShakeOffset_.y;
        BeginMode2D(renderCamera);
    }

    void GameCamera::End() const
    {
        EndMode2D();
    }

    Vector2 GameCamera::ScreenToWorld(Vector2 screenPosition) const
    {
        // 원본 camera_를 사용합니다.
        // 화면 흔들림이 적용된 renderCamera를 쓰면 마우스 조준 좌표도 흔들려서 조작감이 불안정해질 수 있습니다.
        const Vector2 renderPosition = GetScreenToWorld2D(screenPosition, camera_);
        return CoordinateSpace::RenderToWorld(renderPosition);
    }

    Vector2 GameCamera::WorldToScreen(Vector2 worldPosition) const
    {
        return GetWorldToScreen2D(CoordinateSpace::WorldToRender(worldPosition), camera_);
    }

    const Camera2D& GameCamera::RawCamera() const
    {
        return camera_;
    }

    float GameCamera::Zoom() const
    {
        return camera_.zoom;
    }

    void GameCamera::ApplyZoomInput(float mouseWheelMove)
    {
        if (mouseWheelMove == 0.0f)
        {
            return;
        }

        tuning_.zoom *= std::pow(tuning_.zoomStep, mouseWheelMove);
        tuning_.zoom = std::clamp(tuning_.zoom, tuning_.minZoom, tuning_.maxZoom);
        camera_.zoom = tuning_.zoom;
    }

    Vector2 GameCamera::BuildDesiredTarget(
        Vector2 playerWorldPosition,
        Vector2 playerWorldVelocity,
        Vector2 mouseWorldPosition,
        float deltaSeconds)
    {
        const Vector2 rawVelocityLookAhead = {
            std::clamp(
                playerWorldVelocity.x * tuning_.velocityLookAheadSeconds,
                -tuning_.maxLookAheadX,
                tuning_.maxLookAheadX),
            std::clamp(
                playerWorldVelocity.y * tuning_.velocityLookAheadSeconds * tuning_.verticalVelocityLookAheadScale,
                -tuning_.maxLookAheadY,
                tuning_.maxLookAheadY)
        };

        smoothedVelocityLookAhead_ = SmoothVectorTo(
            smoothedVelocityLookAhead_,
            rawVelocityLookAhead,
            tuning_.velocityLookAheadSharpness,
            deltaSeconds);

        // 마우스 조준 방향은 "플레이어 중심 -> 마우스"입니다.
        // 하지만 반동 이동은 그 반대 방향으로 일어나므로, 카메라 bias는 "마우스 -> 플레이어",
        // 즉 playerWorldPosition - mouseWorldPosition 방향을 사용합니다.
        const Vector2 recoilAimVector = {
            playerWorldPosition.x - mouseWorldPosition.x,
            playerWorldPosition.y - mouseWorldPosition.y
        };

        const float aimDistance = math::Length(recoilAimVector);

        const float aimStrength = std::clamp(
            aimDistance / tuning_.recoilAimBiasFullDistance,
            0.0f,
            1.0f);

        const Vector2 recoilAimDirection = math::NormalizeOrZero(recoilAimVector);
        const Vector2 rawRecoilAimBias = {
            recoilAimDirection.x * tuning_.recoilAimBiasDistance * aimStrength,
            recoilAimDirection.y * tuning_.recoilAimBiasDistance * tuning_.recoilAimBiasYScale * aimStrength
        };

        smoothedRecoilAimBias_ = SmoothVectorTo(
            smoothedRecoilAimBias_,
            rawRecoilAimBias,
            tuning_.recoilAimBiasSharpness,
            deltaSeconds);

        return {
            playerWorldPosition.x + smoothedVelocityLookAhead_.x + smoothedRecoilAimBias_.x,
            playerWorldPosition.y + smoothedVelocityLookAhead_.y + smoothedRecoilAimBias_.y
        };
    }

    Vector2 GameCamera::ClampTargetToBounds(Vector2 targetWorld) const
    {
        const float offsetX = static_cast<float>(config::VirtualWidth) * tuning_.offsetRatio.x;
        const float offsetY = static_cast<float>(config::VirtualHeight) * tuning_.offsetRatio.y;

        const float leftVisible = offsetX / camera_.zoom;
        const float rightVisible = (static_cast<float>(config::VirtualWidth) - offsetX) / camera_.zoom;
        const float topVisible = offsetY / camera_.zoom;
        const float bottomVisible = (static_cast<float>(config::VirtualHeight) - offsetY) / camera_.zoom;

        const float minWorldX = worldBounds_.x;
        const float maxWorldX = worldBounds_.x + worldBounds_.width;
        const float minWorldY = worldBounds_.y;
        const float maxWorldY = worldBounds_.y + worldBounds_.height;

        targetWorld.x = ClampWithCollapsedRange(targetWorld.x, minWorldX + leftVisible, maxWorldX - rightVisible);
        targetWorld.y = ClampWithCollapsedRange(targetWorld.y, minWorldY + bottomVisible, maxWorldY - topVisible);

        return targetWorld;
    }

    void GameCamera::UpdateRaylibCamera()
    {
        camera_.offset = {
            static_cast<float>(config::VirtualWidth) * tuning_.offsetRatio.x,
            static_cast<float>(config::VirtualHeight) * tuning_.offsetRatio.y
        };

        camera_.target = CoordinateSpace::WorldToRender(targetWorld_);
        camera_.zoom = tuning_.zoom;
    }

    float GameCamera::SmoothStepTo(float current, float target, float sharpness, float deltaSeconds)
    {
        const float alpha = 1.0f - std::exp(-sharpness * deltaSeconds);
        return current + (target - current) * alpha;
    }

    float GameCamera::MoveTowards(float current, float target, float maxDelta)
    {
        if (target > current)
        {
            return std::min(current + maxDelta, target);
        }

        return std::max(current - maxDelta, target);
    }

    Vector2 GameCamera::SmoothVectorTo(Vector2 current, Vector2 target, float sharpness, float deltaSeconds)
    {
        return {
            SmoothStepTo(current.x, target.x, sharpness, deltaSeconds),
            SmoothStepTo(current.y, target.y, sharpness, deltaSeconds)
        };
    }

}

