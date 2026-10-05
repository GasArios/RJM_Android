// PlayerTerrainEffectResolver.cpp
// - 플레이어와 타일 효과의 접촉을 피해/복구/환경 modifier로 변환합니다.

#include "RecoilJumpMan/Combat/PlayerTerrainEffectResolver.h"

#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/Player/Player.h"

#include <algorithm>

namespace
{
    constexpr float ContactProbePadding = 2.0f;
    constexpr float FootProbeDepth = 4.0f;
    constexpr float HazardDamageAmount = 12.0f;
    constexpr float LavaDamageAmount = 8.0f;
    constexpr float HazardImpactForce = 420.0f;
    constexpr float LavaImpactForce = 140.0f;
}

namespace rjm
{
    PlayerEnvironmentModifiers PlayerTerrainEffectResolver::BuildEnvironment(
        const Player& player,
        const TileMap& tileMap) const
    {
        PlayerEnvironmentModifiers environment;

        std::vector<TileEffectSample> bodyTiles;
        tileMap.CollectEffectTiles(
            player.Hurtbox(),
            TileEffectWater | TileEffectWindUp | TileEffectWindLeft | TileEffectWindRight,
            bodyTiles);

        for (const TileEffectSample& tile : bodyTiles)
        {
            if ((tile.effectFlags & TileEffectWater) != 0)
            {
                environment.inWater = true;
                environment.gravityScale = std::min(environment.gravityScale, 0.45f);
                environment.upwardDragScale = std::max(environment.upwardDragScale, 1.9f);
                environment.apexGravityScale = std::min(environment.apexGravityScale, 0.75f);
                environment.airDragScale = std::max(environment.airDragScale, 1.45f);
                environment.maxRiseSpeedScale = std::min(environment.maxRiseSpeedScale, 0.65f);
                environment.maxFallSpeedScale = std::min(environment.maxFallSpeedScale, 0.45f);
                environment.maxHorizontalSpeedScale = std::min(environment.maxHorizontalSpeedScale, 0.62f);
            }

            if ((tile.effectFlags & TileEffectWindUp) != 0)
            {
                environment.continuousAcceleration.y += 620.0f;
            }

            if ((tile.effectFlags & TileEffectWindLeft) != 0)
            {
                environment.continuousAcceleration.x -= 520.0f;
            }

            if ((tile.effectFlags & TileEffectWindRight) != 0)
            {
                environment.continuousAcceleration.x += 520.0f;
            }
        }

        std::vector<TileEffectSample> surfaceTiles;
        tileMap.CollectEffectTiles(
            BuildFootProbe(player),
            TileEffectIce
            | TileEffectSticky
            | TileEffectReloadSurface
            | TileEffectConveyorLeft
            | TileEffectConveyorRight,
            surfaceTiles);

        for (const TileEffectSample& tile : surfaceTiles)
        {
            if ((tile.effectFlags & TileEffectIce) != 0)
            {
                environment.groundFrictionScale = std::min(environment.groundFrictionScale, 0.32f);
            }

            if ((tile.effectFlags & TileEffectSticky) != 0)
            {
                environment.groundFrictionScale = std::max(environment.groundFrictionScale, 2.35f);
            }

            if ((tile.effectFlags & TileEffectReloadSurface) != 0)
            {
                environment.onReloadSurface = true;
            }

            if ((tile.effectFlags & TileEffectConveyorLeft) != 0)
            {
                environment.surfaceVelocity.x -= 120.0f;
            }

            if ((tile.effectFlags & TileEffectConveyorRight) != 0)
            {
                environment.surfaceVelocity.x += 120.0f;
            }
        }

        return environment;
    }

    PlayerTerrainEffectResult PlayerTerrainEffectResolver::ResolveAfterMovement(
        Player& player,
        const TileMap& tileMap,
        float deltaSeconds,
        double,
        std::vector<HitEvent>& outHitEvents)
    {
        PlayerTerrainEffectResult result;

        std::vector<TileEffectSample> bodyTiles;
        tileMap.CollectEffectTiles(
            player.Hurtbox(),
            TileEffectLava | TileEffectRecovery,
            bodyTiles);

        std::vector<TileEffectSample> hazardTiles;
        tileMap.CollectEffectTiles(
            BuildContactProbe(player),
            TileEffectHazard,
            hazardTiles);

        const bool inHazard = !hazardTiles.empty();
        const bool inLava = ContainsAnyEffect(bodyTiles, TileEffectLava);
        const bool inRecovery = ContainsAnyEffect(bodyTiles, TileEffectRecovery);

        result.canRecordSafePosition = !inHazard && !inLava && !inRecovery;

        if (inRecovery)
        {
            result.recoveryRequest = PlayerTerrainRecoveryRequest::SafePoint;
        }

        if (inHazard)
        {
            for (const TileEffectSample& tile : hazardTiles)
            {
                if ((tile.effectFlags & TileEffectHazard) == 0)
                {
                    continue;
                }

                ApplyTerrainDamage(
                    player,
                    Damage{ HazardDamageAmount, DamageType::Physical, HazardImpactForce },
                    tile.effectId.empty() ? DefinitionId{ "terrain_hazard" } : tile.effectId,
                    tile.rect,
                    "player_hit",
                    8.0f,
                    0.045f,
                    outHitEvents);
                break;
            }
        }

        if (inLava)
        {
            if (!wasInLava_)
            {
                lavaTickRemainingSeconds_ = 0.0f;
            }

            lavaTickRemainingSeconds_ -= std::max(0.0f, deltaSeconds);
            if (lavaTickRemainingSeconds_ <= 0.0f)
            {
                for (const TileEffectSample& tile : bodyTiles)
                {
                    if ((tile.effectFlags & TileEffectLava) == 0)
                    {
                        continue;
                    }

                    const DamageResult damageResult = ApplyTerrainDamage(
                        player,
                        Damage{ LavaDamageAmount, DamageType::Fire, LavaImpactForce },
                        tile.effectId.empty() ? DefinitionId{ "terrain_lava" } : tile.effectId,
                        tile.rect,
                        "player_hit",
                        10.5f,
                        0.035f,
                        outHitEvents);
                    if (damageResult.accepted)
                    {
                        lavaTickRemainingSeconds_ = lavaDamageIntervalSeconds_;
                    }
                    break;
                }
            }

            wasInLava_ = true;
        }
        else
        {
            wasInLava_ = false;
            lavaTickRemainingSeconds_ = 0.0f;
        }

        return result;
    }

    Rectangle PlayerTerrainEffectResolver::BuildContactProbe(const Player& player)
    {
        Rectangle contactProbe = player.Hurtbox();
        contactProbe.x -= ContactProbePadding;
        contactProbe.y -= ContactProbePadding;
        contactProbe.width += ContactProbePadding * 2.0f;
        contactProbe.height += ContactProbePadding * 2.0f;
        return contactProbe;
    }

    void PlayerTerrainEffectResolver::Reset()
    {
        lavaTickRemainingSeconds_ = 0.0f;
        wasInLava_ = false;
    }

    Rectangle PlayerTerrainEffectResolver::BuildFootProbe(const Player& player)
    {
        Rectangle footProbe = player.Hurtbox();
        footProbe.y -= FootProbeDepth * 0.5f;
        footProbe.height = FootProbeDepth;
        return footProbe;
    }

    Vector2 PlayerTerrainEffectResolver::HitNormalFromTile(Rectangle tileRect, Vector2 playerPosition)
    {
        const Vector2 tileCenter = {
            tileRect.x + tileRect.width * 0.5f,
            tileRect.y + tileRect.height * 0.5f
        };
        return math::NormalizeOr(
            math::Subtract(playerPosition, tileCenter),
            { 0.0f, 1.0f });
    }

    bool PlayerTerrainEffectResolver::ContainsAnyEffect(
        const std::vector<TileEffectSample>& tiles,
        TileEffectMask effects)
    {
        for (const TileEffectSample& tile : tiles)
        {
            if ((tile.effectFlags & effects) != 0)
            {
                return true;
            }
        }

        return false;
    }

    WeaponFeedbackProfile PlayerTerrainEffectResolver::BuildTerrainHitFeedback(
        float shakeStrength,
        float hitStopSeconds)
    {
        WeaponFeedbackProfile feedback;
        feedback.hitStopSeconds = hitStopSeconds;
        feedback.hitShakeStrength = shakeStrength;
        feedback.hitShakeSeconds = 0.13f;
        feedback.hitShakeFrequency = 30.0f;
        return feedback;
    }

    DamageResult PlayerTerrainEffectResolver::ApplyTerrainDamage(
        Player& player,
        const Damage& damage,
        const DefinitionId& sourceId,
        Rectangle sourceRect,
        const DefinitionId& hitEffectId,
        float shakeStrength,
        float hitStopSeconds,
        std::vector<HitEvent>& outHitEvents) const
    {
        const Vector2 hitNormal = HitNormalFromTile(sourceRect, player.Position());
        const DamageResult damageResult = player.ApplyDamage(damage, hitNormal);
        if (!damageResult.accepted)
        {
            return damageResult;
        }

        HitEvent event;
        event.sourceId = sourceId;
        event.targetId = "player";
        event.hitEffectId = hitEffectId;
        event.position = player.Position();
        event.normal = hitNormal;
        event.damage = damage;
        event.damageResult = damageResult;
        event.feedback = BuildTerrainHitFeedback(shakeStrength, hitStopSeconds);
        outHitEvents.push_back(event);

        return damageResult;
    }
}

