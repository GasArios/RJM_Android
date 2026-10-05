// RecoilMovementController.cpp
// - 네가 제안한 반동 이동 수식을 실제 코드로 구현한 파일입니다.
// - 이 클래스는 좌표계를 y-up 물리 좌표로 가정합니다.

#include "RecoilJumpMan/Physics/RecoilMovementController.h"

#include <algorithm>
#include <cmath>

namespace rjm
{
    RecoilMovementController::RecoilMovementController() = default;

    RecoilMovementController::RecoilMovementController(RecoilMovementTuning tuning)
        : tuning_(tuning)
    {
    }

    void RecoilMovementController::ApplyRecoilImpulse(Vector2& velocity, Vector2 recoilImpulse) const
    {
        ApplyHorizontalRecoil(velocity, recoilImpulse.x);
        ApplyVerticalRecoil(velocity, recoilImpulse.y);
        ClampVelocity(velocity);
    }

    void RecoilMovementController::ApplyBracedRecoilImpulse(Vector2& velocity, Vector2 recoilImpulse) const
    {
        ApplyRecoilImpulse(velocity, BuildBracedRecoilImpulse(recoilImpulse));
    }

    Vector2 RecoilMovementController::BuildAirborneHeightAdjustedRecoilImpulse(
        Vector2 recoilImpulse,
        float currentY,
        float airborneStartY,
        bool grounded,
        float altitudeDampingResistance) const
    {
        Vector2 adjustedRecoil = recoilImpulse;

        // 이번 장치는 "위로 더 올라가는 힘"만 줄입니다.
        // 수평 반동을 같이 줄이면 공중에서 방향을 고치는 재미까지 죽고,
        // 아래쪽 반동을 줄이면 빠르게 착지해서 재장전 리듬을 회복하는 선택지가 약해집니다.
        if (adjustedRecoil.y > 0.0f)
        {
            adjustedRecoil.y *= AirborneUpwardRecoilEfficiency(
                currentY,
                airborneStartY,
                grounded,
                altitudeDampingResistance);
        }

        return adjustedRecoil;
    }

    float RecoilMovementController::AirborneUpwardRecoilEfficiency(
        float currentY,
        float airborneStartY,
        bool grounded,
        float altitudeDampingResistance) const
    {
        if (!tuning_.airborneUpwardRecoilHeightDampingEnabled || grounded)
        {
            return 1.0f;
        }

        // y-up 좌표계이므로 currentY가 airborneStartY보다 클수록 더 높은 곳에 있다는 뜻입니다.
        // 시작점보다 낮거나 같은 곳에서는 상승 반동을 벌주지 않습니다.
        const float climbedHeight = std::max(0.0f, currentY - airborneStartY);

        // 무기별 고도 감쇠 저항:
        // - 저항이 높은 총은 같은 높이에 있어도 "실제로는 덜 높이 올라온 것처럼" 감쇠 곡선을 늦게 탑니다.
        // - 반동력 자체와 분리되어 있으므로, 어떤 총은 세게 밀지만 고도 제한을 빨리 받고,
        //   어떤 총은 고도 제한을 잘 버티는 식으로 이동 역할을 나눌 수 있습니다.
        const float resistance = std::clamp(altitudeDampingResistance, 0.0f, 1.0f);
        const float resistanceDivisor = 1.0f
            + resistance * std::max(0.0f, tuning_.airborneUpwardRecoilResistanceHeightScale);
        const float effectiveClimbedHeight = climbedHeight / resistanceDivisor;

        return InterpolateAirborneUpwardRecoilEfficiency(effectiveClimbedHeight);
    }

    void RecoilMovementController::ApplyContinuousForces(Vector2& velocity, float deltaSeconds, bool grounded) const
    {
        ApplyContinuousForces(velocity, deltaSeconds, grounded, PlayerEnvironmentModifiers{});
    }

    void RecoilMovementController::ApplyContinuousForces(
        Vector2& velocity,
        float deltaSeconds,
        bool grounded,
        const PlayerEnvironmentModifiers& environment) const
    {
        // y축 지속 물리:
        // - y > 0은 상승, y < 0은 하강입니다.
        const float gravity = tuning_.gravity * std::max(0.0f, environment.gravityScale);
        if (velocity.y > tuning_.apexThreshold)
        {
            // 발사 직후 폭발적 상승 구간:
            // v_y = v_y * Drag_up^dt - Gravity * dt
            const float upwardDrag = std::pow(
                tuning_.upwardDragPerSecond,
                std::max(0.0f, environment.upwardDragScale));
            velocity.y = velocity.y * std::pow(upwardDrag, deltaSeconds)
                - gravity * deltaSeconds;
        }
        else if (velocity.y >= -tuning_.apexThreshold && velocity.y <= tuning_.apexThreshold)
        {
            // 대칭형 정점 체공 구간:
            // v_y = v_y - Gravity * 0.4 * dt
            velocity.y -= gravity
                * tuning_.apexGravityScale
                * std::max(0.0f, environment.apexGravityScale)
                * deltaSeconds;
        }
        else
        {
            // 정직한 낙하 구간:
            // v_y = max(v_y - Gravity * dt, -Vmax)
            const float maxFallSpeed = tuning_.maxFallSpeed
                * std::max(0.0f, environment.maxFallSpeedScale);
            velocity.y = std::max(velocity.y - gravity * deltaSeconds, -maxFallSpeed);
        }

        // x축 지속 마찰:
        // - 공중과 지상에서 서로 다른 감속 계수를 사용합니다.
        const float baseHorizontalDrag = grounded ? tuning_.groundFrictionPerSecond : tuning_.airDragPerSecond;
        const float horizontalDragScale = grounded
            ? environment.groundFrictionScale
            : environment.airDragScale;
        const float horizontalDrag = std::pow(baseHorizontalDrag, std::max(0.0f, horizontalDragScale));
        velocity.x *= std::pow(horizontalDrag, deltaSeconds);

        velocity.x += environment.continuousAcceleration.x * deltaSeconds;
        velocity.y += environment.continuousAcceleration.y * deltaSeconds;

        // Snap to Zero:
        // - 아주 작은 수평 속도는 강제로 0으로 만들어 미세 미끄러짐을 막습니다.
        if (std::fabs(velocity.x) < tuning_.horizontalSnapEpsilon)
        {
            velocity.x = 0.0f;
        }

        ClampVelocity(velocity, environment);
    }

    void RecoilMovementController::ApplyBracedGroundForces(Vector2& velocity, float deltaSeconds) const
    {
        if (deltaSeconds <= 0.0f)
        {
            return;
        }

        velocity.x *= std::pow(tuning_.braceGroundFrictionPerSecond, deltaSeconds);
        if (std::fabs(velocity.x) < tuning_.braceHorizontalSnapEpsilon)
        {
            velocity.x = 0.0f;
        }

        ClampVelocity(velocity);
    }

    const RecoilMovementTuning& RecoilMovementController::Tuning() const
    {
        return tuning_;
    }

    void RecoilMovementController::ApplyVerticalRecoil(Vector2& velocity, float recoilY) const
    {
        if (recoilY > 0.0f)
        {
            // 위로 솟구칠 때:
            // v_y = max(v_y, R) + max(0, min(v_y, R)) * 0.2
            velocity.y = std::max(velocity.y, recoilY)
                + std::max(0.0f, std::min(velocity.y, recoilY)) * tuning_.verticalRecoilSynergyScale;

            // 순간 반동에도 최대 상승 속도 제한을 둡니다.
            velocity.y = std::min(velocity.y, tuning_.maxRiseSpeed);
        }
        else if (recoilY < 0.0f)
        {
            // 아래로 급강하할 때:
            // v_y = min(v_y, R) + min(0, max(v_y, R)) * 0.2
            velocity.y = std::min(velocity.y, recoilY)
                + std::min(0.0f, std::max(velocity.y, recoilY)) * tuning_.verticalRecoilSynergyScale;

            // 하강 반동은 Stomp 최대치로 별도 제한합니다.
            velocity.y = std::max(velocity.y, -tuning_.maxStompSpeed);
        }
    }

    void RecoilMovementController::ApplyHorizontalRecoil(Vector2& velocity, float recoilX) const
    {
        // x축은 순간 반동을 더하는 방식으로 처리합니다.
        velocity.x += recoilX;
        velocity.x = std::clamp(velocity.x, -tuning_.maxHorizontalSpeed, tuning_.maxHorizontalSpeed);
    }

    Vector2 RecoilMovementController::BuildBracedRecoilImpulse(Vector2 recoilImpulse) const
    {
        Vector2 bracedRecoil = recoilImpulse;
        bracedRecoil.x *= tuning_.braceHorizontalRecoilScale;

        if (bracedRecoil.y > 0.0f)
        {
            bracedRecoil.y *= tuning_.braceUpwardRecoilScale;
            if (bracedRecoil.y <= tuning_.braceAnchoredRiseSpeed)
            {
                bracedRecoil.y = 0.0f;
            }
        }
        else if (bracedRecoil.y < 0.0f)
        {
            bracedRecoil.y *= tuning_.braceDownwardRecoilScale;
        }

        return bracedRecoil;
    }

    float RecoilMovementController::InterpolateAirborneUpwardRecoilEfficiency(float climbedHeight) const
    {
        // 튜닝값이 실수로 역순이 되어도 0으로 나누거나 이상한 보간이 나오지 않게,
        // 각 높이는 이전 지점보다 최소 1px 이상 크도록 보정합니다.
        const float fullHeight = std::max(0.0f, tuning_.airborneUpwardRecoilFullEfficiencyHeight);
        const float firstHeight = std::max(fullHeight + 1.0f, tuning_.airborneUpwardRecoilFirstDampingHeight);
        const float secondHeight = std::max(firstHeight + 1.0f, tuning_.airborneUpwardRecoilSecondDampingHeight);
        const float thirdHeight = std::max(secondHeight + 1.0f, tuning_.airborneUpwardRecoilThirdDampingHeight);
        const float minimumHeight = std::max(thirdHeight + 1.0f, tuning_.airborneUpwardRecoilMinimumHeight);

        const float fullScale = 1.0f;
        const float firstScale = std::clamp(tuning_.airborneUpwardRecoilFirstDampingScale, 0.0f, fullScale);
        const float secondScale = std::clamp(tuning_.airborneUpwardRecoilSecondDampingScale, 0.0f, firstScale);
        const float thirdScale = std::clamp(tuning_.airborneUpwardRecoilThirdDampingScale, 0.0f, secondScale);
        const float minimumScale = std::clamp(tuning_.airborneUpwardRecoilMinimumScale, 0.0f, thirdScale);

        if (climbedHeight <= fullHeight)
        {
            return fullScale;
        }

        auto interpolateSegment = [](float value, float fromHeight, float toHeight, float fromScale, float toScale)
        {
            const float t = std::clamp((value - fromHeight) / (toHeight - fromHeight), 0.0f, 1.0f);
            return fromScale + (toScale - fromScale) * t;
        };

        if (climbedHeight <= firstHeight)
        {
            return interpolateSegment(climbedHeight, fullHeight, firstHeight, fullScale, firstScale);
        }

        if (climbedHeight <= secondHeight)
        {
            return interpolateSegment(climbedHeight, firstHeight, secondHeight, firstScale, secondScale);
        }

        if (climbedHeight <= thirdHeight)
        {
            return interpolateSegment(climbedHeight, secondHeight, thirdHeight, secondScale, thirdScale);
        }

        if (climbedHeight <= minimumHeight)
        {
            return interpolateSegment(climbedHeight, thirdHeight, minimumHeight, thirdScale, minimumScale);
        }

        return minimumScale;
    }

    void RecoilMovementController::ClampVelocity(Vector2& velocity) const
    {
        velocity.x = std::clamp(velocity.x, -tuning_.maxHorizontalSpeed, tuning_.maxHorizontalSpeed);
        velocity.y = std::clamp(velocity.y, -tuning_.maxStompSpeed, tuning_.maxRiseSpeed);
    }

    void RecoilMovementController::ClampVelocity(Vector2& velocity, const PlayerEnvironmentModifiers& environment) const
    {
        const float maxHorizontalSpeed = tuning_.maxHorizontalSpeed
            * std::max(0.0f, environment.maxHorizontalSpeedScale);
        const float maxRiseSpeed = tuning_.maxRiseSpeed
            * std::max(0.0f, environment.maxRiseSpeedScale);
        const float maxFallSpeed = tuning_.maxStompSpeed
            * std::max(0.0f, environment.maxFallSpeedScale);

        velocity.x = std::clamp(velocity.x, -maxHorizontalSpeed, maxHorizontalSpeed);
        velocity.y = std::clamp(velocity.y, -maxFallSpeed, maxRiseSpeed);
    }
}

