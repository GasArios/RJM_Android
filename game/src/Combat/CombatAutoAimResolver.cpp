// CombatAutoAimResolver.cpp
// - 우클릭 홀드 중 전투용 자동 락온 대상을 고릅니다.

#include "RecoilJumpMan/Combat/CombatAutoAimResolver.h"

#include "RecoilJumpMan/Core/Config.h"
#include "RecoilJumpMan/Math/VectorMath.h"

#include <algorithm>
#include <cmath>

namespace
{
    Vector2 ResolveProjectileTarget(const rjm::AimAssistCandidate& candidate, bool usePredictedTarget)
    {
        return usePredictedTarget && candidate.hasPredictedWorldPosition
            ? candidate.predictedWorldPosition
            : candidate.worldPosition;
    }

    bool ResolveProjectileLineOfSight(const rjm::AimAssistCandidate& candidate, bool usePredictedTarget)
    {
        return usePredictedTarget && candidate.hasPredictedWorldPosition
            ? candidate.predictedHasLineOfSight
            : candidate.hasLineOfSight;
    }
}

namespace rjm
{
    AimAssistResult CombatAutoAimResolver::Resolve(
        Vector2 playerWorldPosition,
        Vector2 rawWorldTarget,
        const std::vector<AimAssistCandidate>& candidates,
        const CombatAutoAimTuning& tuning,
        bool hasCurrentTarget,
        std::size_t currentTargetStableId) const
    {
        AimAssistResult result = BuildRawResult(rawWorldTarget);
        if (!tuning.enabled || candidates.empty())
        {
            return result;
        }

        const float maxDistance = std::max(0.0f, tuning.maxLockDistanceWorld);
        if (maxDistance <= 0.0f)
        {
            return result;
        }

        const AimAssistCandidate* bestCandidate = nullptr;
        float bestScore = 0.0f;
        float bestDistance = 0.0f;
        float bestCorrectionDegrees = 0.0f;

        for (const AimAssistCandidate& candidate : candidates)
        {
            if (!candidate.enabled)
            {
                continue;
            }

            if (!IsInsideScreenMargin(candidate.screenPosition, tuning.screenMarginPixels))
            {
                continue;
            }

            const bool projectileHasLineOfSight = ResolveProjectileLineOfSight(candidate, tuning.usePredictedTarget);
            if (tuning.requireLineOfSight && !projectileHasLineOfSight)
            {
                continue;
            }

            const float worldDistance = math::Distance(playerWorldPosition, candidate.worldPosition);
            if (worldDistance > maxDistance)
            {
                continue;
            }

            const Vector2 screenCenter = {
                static_cast<float>(config::VirtualWidth) * 0.5f,
                static_cast<float>(config::VirtualHeight) * 0.5f
            };
            const float screenDistance = math::Distance(screenCenter, candidate.screenPosition);
            const float screenMaxDistance = std::sqrt(
                static_cast<float>(config::VirtualWidth * config::VirtualWidth + config::VirtualHeight * config::VirtualHeight)) * 0.5f;

            const float normalizedWorldDistance = worldDistance / maxDistance;
            const float normalizedScreenDistance = screenMaxDistance > 0.0f
                ? screenDistance / screenMaxDistance
                : 0.0f;

            const float priority = std::max(0.01f, candidate.priority);
            float score = (normalizedWorldDistance + normalizedScreenDistance * 0.2f) / priority;

            // 시야선이 막힌 후보는 완전히 버리지는 않되, 열린 후보가 있으면 자연스럽게 밀리게 합니다.
            if (tuning.preferLineOfSight && !projectileHasLineOfSight)
            {
                score += std::max(0.0f, tuning.blockedLinePenalty);
            }

            // 이전 프레임에 이미 붙잡고 있던 대상은 살짝 더 선호합니다.
            // 이 값이 없으면 적 두 마리가 비슷한 거리에 있을 때 락온이 계속 흔들릴 수 있습니다.
            if (hasCurrentTarget
                && candidate.hasStableId
                && candidate.stableId == currentTargetStableId)
            {
                score -= std::max(0.0f, tuning.currentTargetScoreBonus);
            }

            const Vector2 projectileTarget = ResolveProjectileTarget(candidate, tuning.usePredictedTarget);
            const float correctionDegrees = math::DirectionDifferenceDegrees(
                playerWorldPosition,
                rawWorldTarget,
                playerWorldPosition,
                projectileTarget);

            if (!bestCandidate || score < bestScore)
            {
                bestCandidate = &candidate;
                bestScore = score;
                bestDistance = screenDistance;
                bestCorrectionDegrees = correctionDegrees;
            }
        }

        if (!bestCandidate)
        {
            return result;
        }

        result.assisted = true;
        result.lockOn = true;
        result.targetId = bestCandidate->targetId;
        const Vector2 projectileTarget = ResolveProjectileTarget(*bestCandidate, tuning.usePredictedTarget);
        result.projectileWorldTarget = projectileTarget;
        result.recoilWorldTarget = tuning.useLockTargetForRecoil
            ? projectileTarget
            : rawWorldTarget;
        result.targetScreenPosition = bestCandidate->screenPosition;
        result.targetWorldPosition = bestCandidate->worldPosition;
        result.targetStableId = bestCandidate->stableId;
        result.hasTargetStableId = bestCandidate->hasStableId;
        result.hasLineOfSight = ResolveProjectileLineOfSight(*bestCandidate, tuning.usePredictedTarget);
        result.screenDistancePixels = bestDistance;
        result.correctionDegrees = bestCorrectionDegrees;
        return result;
    }

    AimAssistResult CombatAutoAimResolver::BuildRawResult(Vector2 rawWorldTarget)
    {
        AimAssistResult result;
        result.rawWorldTarget = rawWorldTarget;
        result.projectileWorldTarget = rawWorldTarget;
        result.recoilWorldTarget = rawWorldTarget;
        result.targetWorldPosition = rawWorldTarget;
        return result;
    }

    bool CombatAutoAimResolver::IsInsideScreenMargin(Vector2 screenPosition, float marginPixels)
    {
        const float margin = std::max(0.0f, marginPixels);
        return screenPosition.x >= -margin
            && screenPosition.y >= -margin
            && screenPosition.x <= static_cast<float>(config::VirtualWidth) + margin
            && screenPosition.y <= static_cast<float>(config::VirtualHeight) + margin;
    }

}

