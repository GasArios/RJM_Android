#pragma once

// RecoilMovementController.h
// - 플레이어 반동 이동 물리를 담당하는 클래스입니다.
// - 총 발사 순간의 impulse 적용과, 매 프레임 지속되는 중력/저항/마찰을 분리합니다.

#include "RecoilJumpMan/Physics/PlayerEnvironmentModifiers.h"
#include "RecoilJumpMan/Physics/RecoilMovementTuning.h"

#include <raylib.h>

namespace rjm
{
    class RecoilMovementController
    {
    public:
        // 기본 튜닝값으로 컨트롤러를 만듭니다.
        RecoilMovementController();

        // 외부에서 튜닝값을 직접 넘겨 컨트롤러를 만들 수 있습니다.
        explicit RecoilMovementController(RecoilMovementTuning tuning);

        // ApplyRecoilImpulse:
        // - 총 발사 순간의 반동을 velocity에 즉시 적용합니다.
        // - deltaTime이 들어가지 않는 순간적인 속도 변화입니다.
        void ApplyRecoilImpulse(Vector2& velocity, Vector2 recoilImpulse) const;

        // ApplyBracedRecoilImpulse:
        // - 지상 버티기 중 발사 반동을 먼저 흡수한 뒤 velocity에 적용합니다.
        // - 땅 저항만 키우는 방식이 아니라, 발사 순간의 위/아래/좌우 반동 자체를 줄입니다.
        void ApplyBracedRecoilImpulse(Vector2& velocity, Vector2 recoilImpulse) const;

        // BuildAirborneHeightAdjustedRecoilImpulse:
        // - 체공 시작 지점 대비 현재 높이에 따라 위쪽 반동 성분만 점진적으로 줄입니다.
        // - x축 반동과 아래쪽 반동은 건드리지 않아 수평 이동/급강하 조작감을 보존합니다.
        Vector2 BuildAirborneHeightAdjustedRecoilImpulse(
            Vector2 recoilImpulse,
            float currentY,
            float airborneStartY,
            bool grounded,
            float altitudeDampingResistance = 0.0f) const;

        // AirborneUpwardRecoilEfficiency:
        // - 현재 높이에서 위쪽 반동이 몇 % 효율로 적용되는지 계산합니다.
        // - 디버그 UI나 궤적 미리보기에서도 같은 값을 읽을 수 있도록 public으로 둡니다.
        float AirborneUpwardRecoilEfficiency(
            float currentY,
            float airborneStartY,
            bool grounded,
            float altitudeDampingResistance = 0.0f) const;

        // ApplyContinuousForces:
        // - 공중/지상 상태에 따라 매 프레임 중력, 상승 저항, 수평 마찰을 적용합니다.
        void ApplyContinuousForces(Vector2& velocity, float deltaSeconds, bool grounded) const;

        // ApplyContinuousForces:
        // - 물/바람/얼음/점액 같은 지형 효과 보정을 함께 적용합니다.
        void ApplyContinuousForces(
            Vector2& velocity,
            float deltaSeconds,
            bool grounded,
            const PlayerEnvironmentModifiers& environment) const;

        // ApplyBracedGroundForces:
        // - 버티기를 누르고 지상에 있는 동안 남은 수평 속도를 더 강하게 줄입니다.
        void ApplyBracedGroundForces(Vector2& velocity, float deltaSeconds) const;

        // Tuning:
        // - 현재 튜닝값을 읽기 전용으로 반환합니다.
        const RecoilMovementTuning& Tuning() const;

    private:
        // ApplyVerticalRecoil:
        // - 네가 제안한 y축 순간 반동 수식을 구현합니다.
        void ApplyVerticalRecoil(Vector2& velocity, float recoilY) const;

        // ApplyHorizontalRecoil:
        // - x축 반동 impulse를 적용하고 최대 수평 속도로 제한합니다.
        void ApplyHorizontalRecoil(Vector2& velocity, float recoilX) const;

        // BuildBracedRecoilImpulse:
        // - 원본 반동 벡터를 버티기 중 실제로 적용할 반동 벡터로 변환합니다.
        Vector2 BuildBracedRecoilImpulse(Vector2 recoilImpulse) const;

        // InterpolateAirborneUpwardRecoilEfficiency:
        // - 체공 시작 지점 대비 상승 높이를 받아 튜닝 지점 사이를 선형으로 보간합니다.
        float InterpolateAirborneUpwardRecoilEfficiency(float climbedHeight) const;

        // ClampVelocity:
        // - 최종 속도가 설정한 한계를 넘지 않도록 제한합니다.
        void ClampVelocity(Vector2& velocity) const;

        // ClampVelocity:
        // - 환경별 속도 상한 배율까지 반영해 제한합니다.
        void ClampVelocity(Vector2& velocity, const PlayerEnvironmentModifiers& environment) const;

        // 반동 이동 튜닝값 묶음입니다.
        RecoilMovementTuning tuning_;
    };
}

