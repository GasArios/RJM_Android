// WorldInteractionResolver.cpp
// - 월드 상호작용 후보를 고르고 입력에 따라 활성화 결과를 만듭니다.

#include "RecoilJumpMan/World/WorldInteractionResolver.h"

#include "RecoilJumpMan/World/RespawnPoint.h"
#include "RecoilJumpMan/World/TransitionTrigger.h"
#include "RecoilJumpMan/World/WorldMap.h"

#include <algorithm>

namespace rjm
{
    WorldInteractionCandidate WorldInteractionResolver::FindBestCandidate(
        const WorldMap& world,
        Vector2 playerPosition) const
    {
        WorldInteractionCandidate best;

        for (const TransitionTrigger& transition : world.Transitions())
        {
            if (!transition.Contains(playerPosition) || !transition.RequiresInteraction())
            {
                continue;
            }

            WorldInteractionCandidate candidate;
            candidate.type = WorldInteractionType::Transition;
            candidate.id = transition.id;
            candidate.position = RectCenter(transition.bounds);
            candidate.requiresInput = true;
            candidate.priority = 1000;
            candidate.transition = &transition;

            if (!IsValid(best) || candidate.priority > best.priority)
            {
                best = candidate;
            }
        }

        const float radiusSquared = respawnInteractionRadius_ * respawnInteractionRadius_;
        for (const RespawnPoint& respawnPoint : world.RespawnPoints())
        {
            const float distanceSquared = DistanceSquared(playerPosition, respawnPoint.position);
            if (distanceSquared > radiusSquared)
            {
                continue;
            }

            if (!IsRespawnPointInteractable(respawnPoint) && !IsRespawnPointAutomatic(respawnPoint))
            {
                continue;
            }

            WorldInteractionCandidate candidate;
            candidate.type = WorldInteractionType::RespawnPoint;
            candidate.id = respawnPoint.id;
            candidate.position = respawnPoint.position;
            candidate.requiresInput = !IsRespawnPointAutomatic(respawnPoint);
            candidate.priority = respawnPoint.priority + (candidate.requiresInput ? 400 : 200);
            candidate.respawnPoint = &respawnPoint;

            // 같은 priority라면 더 가까운 부활 앵커를 고릅니다.
            if (!IsValid(best)
                || candidate.priority > best.priority
                || (candidate.priority == best.priority
                    && distanceSquared < DistanceSquared(playerPosition, best.position)))
            {
                best = candidate;
            }
        }

        return best;
    }

    WorldInteractionResult WorldInteractionResolver::Resolve(
        const WorldMap& world,
        Vector2 playerPosition,
        bool interactPressed) const
    {
        WorldInteractionResult result;

        for (const TransitionTrigger& transition : world.Transitions())
        {
            if (!transition.Contains(playerPosition))
            {
                continue;
            }

            if (transition.kind == TransitionKind::Touch)
            {
                result.candidate.type = WorldInteractionType::Transition;
                result.candidate.id = transition.id;
                result.candidate.position = RectCenter(transition.bounds);
                result.candidate.requiresInput = false;
                result.candidate.priority = 1200;
                result.candidate.transition = &transition;
                result.activated = true;
                result.transition = &transition;
                return result;
            }
        }

        result.candidate = FindBestCandidate(world, playerPosition);
        if (!IsValid(result.candidate))
        {
            return result;
        }

        const bool canActivate = !result.candidate.requiresInput || interactPressed;
        if (!canActivate)
        {
            return result;
        }

        result.activated = true;
        result.transition = result.candidate.transition;
        result.respawnPoint = result.candidate.respawnPoint;
        result.shouldBindRespawnPoint = result.respawnPoint != nullptr;
        return result;
    }

    bool WorldInteractionResolver::IsValid(const WorldInteractionCandidate& candidate)
    {
        return candidate.type != WorldInteractionType::None;
    }

    bool WorldInteractionResolver::IsRespawnPointAutomatic(const RespawnPoint& respawnPoint)
    {
        return respawnPoint.kind == RespawnPointKind::FieldEntrance
            || respawnPoint.kind == RespawnPointKind::Debug;
    }

    bool WorldInteractionResolver::IsRespawnPointInteractable(const RespawnPoint& respawnPoint)
    {
        return respawnPoint.kind == RespawnPointKind::Town
            || respawnPoint.kind == RespawnPointKind::Camp
            || respawnPoint.kind == RespawnPointKind::BossGate;
    }

    float WorldInteractionResolver::DistanceSquared(Vector2 a, Vector2 b)
    {
        const float dx = a.x - b.x;
        const float dy = a.y - b.y;
        return dx * dx + dy * dy;
    }

    Vector2 WorldInteractionResolver::RectCenter(Rectangle rect)
    {
        return {
            rect.x + rect.width * 0.5f,
            rect.y + rect.height * 0.5f
        };
    }
}
