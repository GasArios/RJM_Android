#pragma once

// WorldInteractionResolver.h
// - 플레이어 위치와 입력을 바탕으로 현재 월드에서 가능한 상호작용을 고릅니다.
// - 이 클래스는 "무엇이 활성화되었는가"만 판단하고, 실제 텔레포트/부활 지점 등록은 GameplayScene이 실행합니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <raylib.h>

namespace rjm
{
    class WorldMap;
    struct RespawnPoint;
    struct TransitionTrigger;

    enum class WorldInteractionType
    {
        None,
        Transition,
        RespawnPoint
    };

    struct WorldInteractionCandidate
    {
        WorldInteractionType type = WorldInteractionType::None;
        DefinitionId id;
        Vector2 position = { 0.0f, 0.0f };
        bool requiresInput = false;
        int priority = 0;

        const TransitionTrigger* transition = nullptr;
        const RespawnPoint* respawnPoint = nullptr;
    };

    struct WorldInteractionResult
    {
        WorldInteractionCandidate candidate;
        bool activated = false;

        const TransitionTrigger* transition = nullptr;
        const RespawnPoint* respawnPoint = nullptr;

        bool shouldBindRespawnPoint = false;
    };

    class WorldInteractionResolver
    {
    public:
        WorldInteractionCandidate FindBestCandidate(const WorldMap& world, Vector2 playerPosition) const;

        WorldInteractionResult Resolve(
            const WorldMap& world,
            Vector2 playerPosition,
            bool interactPressed) const;

    private:
        static bool IsValid(const WorldInteractionCandidate& candidate);
        static bool IsRespawnPointAutomatic(const RespawnPoint& respawnPoint);
        static bool IsRespawnPointInteractable(const RespawnPoint& respawnPoint);
        static float DistanceSquared(Vector2 a, Vector2 b);
        static Vector2 RectCenter(Rectangle rect);

        float respawnInteractionRadius_ = 72.0f;
    };
}

