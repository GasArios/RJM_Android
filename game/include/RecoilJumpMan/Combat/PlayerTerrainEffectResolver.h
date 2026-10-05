#pragma once

// PlayerTerrainEffectResolver.h
// - 플레이어가 타일 효과 볼륨/표면과 만났을 때의 결과를 해석합니다.
// - TileMap은 효과가 어디에 있는지만 알고, 피해/복구/환경 modifier 해석은 이 클래스가 담당합니다.

#include "RecoilJumpMan/Combat/HitEvent.h"
#include "RecoilJumpMan/Physics/PlayerEnvironmentModifiers.h"
#include "RecoilJumpMan/World/TileMap.h"

#include <vector>

namespace rjm
{
    class Player;

    enum class PlayerTerrainRecoveryRequest
    {
        None,
        SafePoint
    };

    struct PlayerTerrainEffectResult
    {
        // 위험 지형 위에서는 마지막 안전 위치로 기록하지 않게 합니다.
        bool canRecordSafePosition = true;

        // 낙하/즉사/맵 밖 처리처럼 체력 피해가 아닌 복구 요청입니다.
        PlayerTerrainRecoveryRequest recoveryRequest = PlayerTerrainRecoveryRequest::None;
    };

    class PlayerTerrainEffectResolver
    {
    public:
        // BuildEnvironment:
        // - 현재 플레이어가 겹치거나 밟고 있는 타일 효과를 물리 modifier로 변환합니다.
        PlayerEnvironmentModifiers BuildEnvironment(const Player& player, const TileMap& tileMap) const;

        // ResolveAfterMovement:
        // - 이동이 끝난 플레이어 위치 기준으로 피해/복구/안전 위치 기록 가능 여부를 판단합니다.
        PlayerTerrainEffectResult ResolveAfterMovement(
            Player& player,
            const TileMap& tileMap,
            float deltaSeconds,
            double currentTimeSeconds,
            std::vector<HitEvent>& outHitEvents);

        // Reset:
        // - 레벨 전환, 부활, 안전 지점 복구 후 타일 지속 피해 타이머를 초기화합니다.
        void Reset();

    private:
        static Rectangle BuildContactProbe(const Player& player);
        static Rectangle BuildFootProbe(const Player& player);
        static Vector2 HitNormalFromTile(Rectangle tileRect, Vector2 playerPosition);
        static bool ContainsAnyEffect(const std::vector<TileEffectSample>& tiles, TileEffectMask effects);
        static WeaponFeedbackProfile BuildTerrainHitFeedback(float shakeStrength, float hitStopSeconds);

        DamageResult ApplyTerrainDamage(
            Player& player,
            const Damage& damage,
            const DefinitionId& sourceId,
            Rectangle sourceRect,
            const DefinitionId& hitEffectId,
            float shakeStrength,
            float hitStopSeconds,
            std::vector<HitEvent>& outHitEvents) const;

        float lavaDamageIntervalSeconds_ = 0.75f;
        float lavaTickRemainingSeconds_ = 0.0f;
        bool wasInLava_ = false;
    };
}

