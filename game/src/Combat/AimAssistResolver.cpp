// AimAssistResolver.cpp
// - 마우스 포인터 주변 후보 중 가장 자연스러운 조준 보정 대상을 고릅니다.

#include "RecoilJumpMan/Combat/AimAssistResolver.h"

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

    Vector2 ResolveScoringScreenPosition(
        const rjm::AimAssistCandidate& candidate,
        Vector2 mouseScreenPosition,
        bool usePredictedTarget)
    {
        if (!usePredictedTarget || !candidate.hasPredictedWorldPosition)
        {
            return candidate.screenPosition;
        }

        const float currentDistanceSq = rjm::math::DistanceSquared(mouseScreenPosition, candidate.screenPosition);
        const float predictedDistanceSq = rjm::math::DistanceSquared(mouseScreenPosition, candidate.predictedScreenPosition);
        return predictedDistanceSq < currentDistanceSq
            ? candidate.predictedScreenPosition
            : candidate.screenPosition;
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
    AimAssistResult AimAssistResolver::Resolve(
        Vector2 playerWorldPosition,
        Vector2 rawWorldTarget,
        Vector2 mouseScreenPosition,
        const std::vector<AimAssistCandidate>& candidates,
        const AimAssistTuning& tuning) const
    {
        AimAssistResult result = BuildRawResult(rawWorldTarget);
        if (!tuning.enabled || candidates.empty())
        {
            return result;
        }

        const float baseRadius = std::max(0.0f, tuning.screenRadiusPixels);
        const float radiusScale = std::max(0.0f, tuning.radiusScale);
        const float maxCorrectionDegrees = std::max(0.0f, tuning.maxCorrectionDegrees);

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

            const float candidateRadius = candidate.screenRadiusPixels > 0.0f
                ? candidate.screenRadiusPixels
                : baseRadius;
            const float effectiveRadius = candidateRadius * radiusScale;
            if (effectiveRadius <= 0.0f)
            {
                continue;
            }

            const Vector2 scoringScreenPosition = ResolveScoringScreenPosition(
                candidate,
                mouseScreenPosition,
                tuning.usePredictedTarget);
            const float distanceSquared = math::DistanceSquared(mouseScreenPosition, scoringScreenPosition);
            if (distanceSquared > effectiveRadius * effectiveRadius)
            {
                continue;
            }

            const Vector2 projectileTarget = ResolveProjectileTarget(candidate, tuning.usePredictedTarget);
            const float correctionDegrees = math::DirectionDifferenceDegrees(
                playerWorldPosition,
                rawWorldTarget,
                playerWorldPosition,
                projectileTarget);
            if (correctionDegrees > maxCorrectionDegrees)
            {
                continue;
            }

            const float distance = std::sqrt(distanceSquared);
            const float priority = std::max(0.01f, candidate.priority);
            const float normalizedDistance = distance / effectiveRadius;
            const float normalizedCorrection = maxCorrectionDegrees > 0.0f
                ? correctionDegrees / maxCorrectionDegrees
                : 0.0f;

            // score:
            // - 가까운 후보를 우선하되, priority가 높은 후보는 조금 더 쉽게 선택되게 합니다.
            // - 보정 각도도 약간 반영해 갑작스러운 방향 변경보다 자연스러운 보정을 선호합니다.
            const float score = (normalizedDistance + normalizedCorrection * 0.35f) / priority;
            if (!bestCandidate || score < bestScore)
            {
                bestCandidate = &candidate;
                bestScore = score;
                bestDistance = distance;
                bestCorrectionDegrees = correctionDegrees;
            }
        }

        if (!bestCandidate)
        {
            return result;
        }

        result.assisted = true;
        result.targetId = bestCandidate->targetId;
        const Vector2 projectileTarget = ResolveProjectileTarget(*bestCandidate, tuning.usePredictedTarget);
        result.projectileWorldTarget = projectileTarget;
        result.recoilWorldTarget = tuning.useAssistedTargetForRecoil
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

    AimAssistResult AimAssistResolver::BuildRawResult(Vector2 rawWorldTarget)
    {
        AimAssistResult result;
        result.rawWorldTarget = rawWorldTarget;
        result.projectileWorldTarget = rawWorldTarget;
        result.recoilWorldTarget = rawWorldTarget;
        result.targetWorldPosition = rawWorldTarget;
        return result;
    }

}

