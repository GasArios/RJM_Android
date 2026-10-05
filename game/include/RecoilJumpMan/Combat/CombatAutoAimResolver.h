#pragma once

// CombatAutoAimResolver.h
// - 우클릭 홀드 중 전투용 자동 락온 대상을 고르는 클래스입니다.
// - 기존 AimAssistResolver가 "마우스 포인터 근처 후보"를 고른다면,
//   이 클래스는 "플레이어가 지금 전투 대상으로 삼기 좋은 후보"를 고릅니다.

#include "RecoilJumpMan/Combat/AimAssistResolver.h"

#include <cstddef>
#include <vector>

namespace rjm
{
    struct CombatAutoAimTuning
    {
        // enabled:
        // - false이면 우클릭을 누르고 있어도 전투 락온을 사용하지 않습니다.
        bool enabled = true;

        // maxLockDistanceWorld:
        // - 플레이어와 대상 사이의 최대 락온 거리입니다.
        // - 너무 멀리 있는 적에게 강제로 조준이 붙으면 이동 의도를 읽기 어려워지므로 월드 거리 제한을 둡니다.
        float maxLockDistanceWorld = 900.0f;

        // screenMarginPixels:
        // - 화면 안 또는 카메라 근처로 인정할 여유 픽셀입니다.
        // - 화면 밖으로 살짝 나간 적도 계속 락온되게 해 갑작스러운 끊김을 줄입니다.
        float screenMarginPixels = 140.0f;

        // preferLineOfSight:
        // - true이면 벽에 막히지 않은 후보를 더 선호합니다.
        bool preferLineOfSight = true;

        // requireLineOfSight:
        // - true이면 벽에 막힌 후보는 아예 선택하지 않습니다.
        // - 현재는 "가능하면 시야선" 설계라 false를 기본값으로 둡니다.
        bool requireLineOfSight = false;

        // blockedLinePenalty:
        // - preferLineOfSight가 true일 때, 시야선이 막힌 후보에게 더하는 점수 패널티입니다.
        float blockedLinePenalty = 0.65f;

        // currentTargetScoreBonus:
        // - 기존 락온 대상이 여전히 유효하면 점수를 낮춰 계속 붙잡게 만드는 보너스입니다.
        // - 적 둘의 거리가 비슷할 때 매 프레임 타깃이 튀는 일을 줄입니다.
        float currentTargetScoreBonus = 0.35f;

        // useLockTargetForRecoil:
        // - true이면 탄 방향과 반동 방향이 모두 락온 대상 기준으로 계산됩니다.
        // - false이면 탄은 락온 대상에게 가지만, 반동은 원래 마우스 방향을 유지합니다.
        bool useLockTargetForRecoil = true;

        // usePredictedTarget:
        // - true이면 후보가 가진 예측 좌표를 탄환 목표로 사용할 수 있습니다.
        // - 우클릭 락온은 좌클릭 보정보다 더 강하게 예측해 신뢰성을 높입니다.
        bool usePredictedTarget = false;
    };

    class CombatAutoAimResolver
    {
    public:
        // Resolve:
        // - 후보 목록에서 우클릭 전투 락온 대상을 고릅니다.
        // - 후보가 없거나 조건을 통과하지 못하면 rawWorldTarget 그대로 반환합니다.
        AimAssistResult Resolve(
            Vector2 playerWorldPosition,
            Vector2 rawWorldTarget,
            const std::vector<AimAssistCandidate>& candidates,
            const CombatAutoAimTuning& tuning,
            bool hasCurrentTarget,
            std::size_t currentTargetStableId) const;

    private:
        static AimAssistResult BuildRawResult(Vector2 rawWorldTarget);
        static bool IsInsideScreenMargin(Vector2 screenPosition, float marginPixels);
    };
}

