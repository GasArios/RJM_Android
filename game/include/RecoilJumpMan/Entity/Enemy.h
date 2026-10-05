#pragma once

// Enemy.h
// - SpawnPoint를 기반으로 미리 만들어지는 적 객체입니다.
// - isActive는 "플레이어 근처라 업데이트 대상인가", isAlive는 "현재 살아있는가"를 뜻합니다.

#include "RecoilJumpMan/Combat/Health.h"
#include "RecoilJumpMan/Combat/Damage.h"
#include "RecoilJumpMan/Data/EnemyDefinition.h"
#include "RecoilJumpMan/Entity/Entity.h"
#include "RecoilJumpMan/Physics/TileCollisionResolver.h"
#include "RecoilJumpMan/World/SpawnPoint.h"

namespace rjm
{
    class TileMap;

    class Enemy final : public Entity
    {
    public:
        Enemy();

        void Initialize(const SpawnPoint& spawnPoint, const EnemyDefinition& definition);
        void Update(
            float deltaSeconds,
            double currentTimeSeconds,
            const EnemyAiContext& aiContext,
            const TileMap& tileMap);
        void Draw() const override;

        bool IsAlive() const;

        // Hurtbox:
        // - 투사체가 맞을 수 있는 피격 박스를 반환합니다.
        // - 현재 디버그 적은 하나의 사각형 Hurtbox만 쓰지만,
        //   나중에 보스 약점/방패/부위별 판정으로 확장할 수 있습니다.
        Rectangle Hurtbox() const;

        // ContactHitbox:
        // - 플레이어에게 접촉 피해를 줄 수 있는 공격 판정입니다.
        // - 현재는 Hurtbox와 같은 사각형을 쓰지만, 나중에 검/돌진/보스 공격 판정과 분리할 수 있습니다.
        Rectangle ContactHitbox() const;

        // CanDamagePlayerOnContact:
        // - 현재 AI 상태와 데이터 기준으로 플레이어 접촉 피해를 줄 수 있는지 반환합니다.
        bool CanDamagePlayerOnContact() const;

        // ContactDamage:
        // - 플레이어에게 전달할 접촉 피해 데이터를 만듭니다.
        Damage ContactDamage() const;

        // ApplyDamage:
        // - 외부 전투 시스템이 피해를 요청할 때 사용하는 입구입니다.
        // - 실제 체력 감소, 사망 처리, 무적/방어/약점 보정은 이 함수 안에서 처리하는 것이 자연스럽습니다.
        DamageResult ApplyDamage(const Damage& damage, double currentTimeSeconds);
        void Kill(double currentTimeSeconds);
        void Respawn();

        const SpawnPoint& Spawn() const;

        // Definition:
        // - 이 적의 데이터 정의를 반환합니다.
        // - HitEvent의 targetId나 향후 적 타입별 연출 분기에 사용할 수 있습니다.
        const EnemyDefinition& Definition() const;

        // AllowsAimAssist:
        // - 현재 이 적이 조준 보정 후보로 들어갈 수 있는지 반환합니다.
        // - 살아 있고 active이며, 데이터 정의에서 보정을 허용한 경우에만 true입니다.
        bool AllowsAimAssist() const;

        // AimAssistPoint:
        // - 조준 보정이 탄을 끌어당길 월드 좌표입니다.
        // - 현재는 Hurtbox 중심이지만, 나중에는 머리/약점/보스 코어 같은 지점으로 바꿀 수 있습니다.
        Vector2 AimAssistPoint() const;

        // AimAssistRadiusPixels:
        // - 이 적에게 적용할 화면 기준 보정 반경입니다.
        // - EnemyDefinition의 값이 0 이하이면 무기 기본값을 쓰라는 의미로 그대로 반환합니다.
        float AimAssistRadiusPixels() const;

        // AimAssistPriority:
        // - 후보 선택에서 사용할 우선도입니다.
        // - 높은 값일수록 같은 거리에서 더 쉽게 선택됩니다.
        float AimAssistPriority() const;

        // ApplyCrowdSeparationPush:
        // - EnemyPool의 군중 분리 후처리가 적을 아주 조금 밀어낼 때 사용하는 입구입니다.
        // - push는 직접 위치 보정량이며, 내부에서 타일 충돌을 다시 통과시켜 벽 안으로 들어가지 않게 합니다.
        void ApplyCrowdSeparationPush(Vector2 push, const TileMap& tileMap);

    private:
        struct EnemyTerrainProbe
        {
            bool grounded = false;
            bool hitWall = false;
            bool hitCeiling = false;
            bool wallAhead = false;
            bool floorAhead = false;
            bool ceilingAbove = false;
        };

        void ResetAiState();
        void UpdateAi(float deltaSeconds, const EnemyAiContext& aiContext, const TileMap& tileMap);
        void ChangeAiState(EnemyAiState state);
        void BeginWander();
        void BeginAttack(const EnemyAiContext& aiContext);
        void UpdateMovement(float deltaSeconds, const TileMap& tileMap);
        void RefreshTerrainProbe(const TileMap& tileMap);
        float BodySize() const;
        float HalfBodySize() const;
        bool CanSeePlayer(const EnemyAiContext& aiContext, const TileMap& tileMap) const;
        bool HasRecentPlayerSighting() const;
        Vector2 ResolvePressureTarget(const EnemyAiContext& aiContext) const;
        float ResolveTelegraphSeconds() const;
        float ResolveRecoverSeconds() const;
        bool CanStartAttack(Vector2 pressureTarget) const;
        bool ShouldReturnToSpawn() const;
        void FaceToward(float targetX);

        SpawnPoint spawnPoint_;
        EnemyDefinition definition_;
        Health health_;
        bool alive_ = true;
        double deathTimeSeconds_ = -1.0;

        TileCollisionResolver tileCollision_;
        EnemyTerrainProbe terrainProbe_;

        EnemyAiState aiState_ = EnemyAiState::Idle;
        float aiStateSeconds_ = 0.0f;
        float attackCooldownRemaining_ = 0.0f;
        float hopCooldownRemaining_ = 0.0f;
        float facingDirection_ = 1.0f;
        float wanderDirection_ = 1.0f;
        int wanderCycleIndex_ = 0;
        bool playerVisible_ = false;
        float timeSinceLastSeenPlayer_ = 9999.0f;
        Vector2 lastSeenPlayerPosition_ = { 0.0f, 0.0f };
        Vector2 lastKnownPlayerGroundPosition_ = { 0.0f, 0.0f };
    };
}

