// Enemy.cpp
// - Enemy의 활성화/생존/리스폰과 임시 렌더링 구현부입니다.

#include "RecoilJumpMan/Entity/Enemy.h"

#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/Physics/CoordinateSpace.h"
#include "RecoilJumpMan/World/TileLineOfSight.h"
#include "RecoilJumpMan/World/TileMap.h"

#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace
{
    constexpr float EnemyGroundGravity = 900.0f;
    constexpr float EnemyWallProbeDistance = 6.0f;
    constexpr float EnemyFloorProbeDistance = 8.0f;
    constexpr float EnemyCeilingProbeDistance = 4.0f;

    Color ColorForAiState(rjm::EnemyAiState state)
    {
        switch (state)
        {
        case rjm::EnemyAiState::Alert:
            return Color{ 245, 150, 75, 255 };
        case rjm::EnemyAiState::Telegraph:
            return Color{ 255, 220, 95, 255 };
        case rjm::EnemyAiState::Attack:
            return Color{ 225, 80, 130, 255 };
        case rjm::EnemyAiState::Recover:
            return Color{ 150, 150, 170, 255 };
        case rjm::EnemyAiState::Return:
            return Color{ 110, 160, 230, 255 };
        case rjm::EnemyAiState::Wander:
            return Color{ 180, 105, 90, 255 };
        case rjm::EnemyAiState::Idle:
        case rjm::EnemyAiState::Approach:
        default:
            return Color{ 210, 70, 80, 255 };
        }
    }

    float ReactionSeconds(rjm::EnemyReactionSpeed speed)
    {
        switch (speed)
        {
        case rjm::EnemyReactionSpeed::Slow:
            return 0.45f;
        case rjm::EnemyReactionSpeed::Fast:
            return 0.12f;
        case rjm::EnemyReactionSpeed::Normal:
        default:
            return 0.25f;
        }
    }

    constexpr float ContactImpactForce = 500.0f;
}

namespace rjm
{
    Enemy::Enemy()
        : health_(1.0f)
    {
        SetActive(false);
    }

    void Enemy::Initialize(const SpawnPoint& spawnPoint, const EnemyDefinition& definition)
    {
        spawnPoint_ = spawnPoint;
        definition_ = definition;
        health_.SetMaximum(definition_.maxHealth, true);
        position_ = spawnPoint_.position;
        velocity_ = { 0.0f, 0.0f };
        alive_ = true;
        deathTimeSeconds_ = -1.0;
        terrainProbe_ = EnemyTerrainProbe{};
        ResetAiState();
        SetActive(false);
    }

    void Enemy::Update(
        float deltaSeconds,
        double currentTimeSeconds,
        const EnemyAiContext& aiContext,
        const TileMap& tileMap)
    {
        if (!IsActive())
        {
            return;
        }

        if (!alive_)
        {
            if (currentTimeSeconds >= deathTimeSeconds_ + spawnPoint_.respawnSeconds)
            {
                Respawn();
            }

            return;
        }

        RefreshTerrainProbe(tileMap);
        UpdateAi(deltaSeconds, aiContext, tileMap);
        UpdateMovement(deltaSeconds, tileMap);
    }

    void Enemy::Draw() const
    {
        if (!IsActive() || !alive_)
        {
            return;
        }

        const Vector2 renderPosition = CoordinateSpace::WorldToRender(position_);
        const float size = BodySize();
        const float halfSize = size * 0.5f;
        const Rectangle body = { renderPosition.x - halfSize, renderPosition.y - halfSize, size, size };
        DrawRectangleRec(body, ColorForAiState(aiState_));
        DrawRectangleLinesEx(body, 2.0f, Color{ 255, 150, 160, 255 });

        if (aiState_ == EnemyAiState::Telegraph)
        {
            DrawCircleLines(
                static_cast<int>(renderPosition.x),
                static_cast<int>(renderPosition.y),
                22.0f,
                Color{ 255, 235, 145, 220 });
        }
    }

    bool Enemy::IsAlive() const
    {
        return alive_;
    }

    Rectangle Enemy::Hurtbox() const
    {
        // 현재 일반 적은 화면에 그리는 사각형과 같은 크기의 Hurtbox를 씁니다.
        // position_은 중심 좌표이므로, 왼쪽 아래 기준 Rectangle을 만들기 위해 halfSize를 뺍니다.
        const float size = BodySize();
        const float halfSize = size * 0.5f;
        return {
            position_.x - halfSize,
            position_.y - halfSize,
            size,
            size
        };
    }

    Rectangle Enemy::ContactHitbox() const
    {
        return Hurtbox();
    }

    bool Enemy::CanDamagePlayerOnContact() const
    {
        if (!IsActive() || !alive_ || definition_.contactDamage <= 0.0f)
        {
            return false;
        }

        switch (definition_.ai.attackStyle)
        {
        case EnemyAttackStyle::Contact:
            return true;
        case EnemyAttackStyle::Lunge:
        case EnemyAttackStyle::JumpPounce:
            return aiState_ == EnemyAiState::Attack;
        case EnemyAttackStyle::Projectile:
        case EnemyAttackStyle::Area:
        case EnemyAttackStyle::Grab:
        case EnemyAttackStyle::Shield:
        default:
            return false;
        }
    }

    Damage Enemy::ContactDamage() const
    {
        return Damage{
            definition_.contactDamage,
            DamageType::Physical,
            ContactImpactForce
        };
    }

    DamageResult Enemy::ApplyDamage(const Damage& damage, double currentTimeSeconds)
    {
        // DamageResult는 "요청된 피해"와 "실제 적용된 결과"를 함께 돌려줍니다.
        // 지금은 방어력/내성이 없지만, 나중에 보정이 들어와도 호출자 구조는 유지됩니다.
        DamageResult result;
        result.requestedAmount = damage.amount;
        result.remainingHealth = health_.Current();

        // 비활성 적이나 이미 죽은 적은 피해를 받지 않습니다.
        if (!IsActive() || !alive_)
        {
            return result;
        }

        result.accepted = true;

        // 현재는 음수 피해를 허용하지 않고 0 이상으로 보정합니다.
        // 속성 내성/방어력/약점 배율은 이 줄 주변에 들어갈 수 있습니다.
        result.appliedAmount = std::max(0.0f, damage.amount);

        health_.Damage(result.appliedAmount);
        result.remainingHealth = health_.Current();
        result.killed = health_.IsDead();

        // 사망 처리도 Enemy 내부에서 확정합니다.
        // 이렇게 해야 리스폰 타이머와 alive_ 상태가 외부 전투 코드에 흩어지지 않습니다.
        if (result.killed)
        {
            Kill(currentTimeSeconds);
        }

        return result;
    }

    void Enemy::Kill(double currentTimeSeconds)
    {
        alive_ = false;
        deathTimeSeconds_ = currentTimeSeconds;
        health_.Damage(health_.Current());
    }

    void Enemy::Respawn()
    {
        alive_ = true;
        position_ = spawnPoint_.position;
        velocity_ = { 0.0f, 0.0f };
        terrainProbe_ = EnemyTerrainProbe{};
        health_.SetMaximum(definition_.maxHealth, true);
        ResetAiState();
    }

    const SpawnPoint& Enemy::Spawn() const
    {
        return spawnPoint_;
    }

    const EnemyDefinition& Enemy::Definition() const
    {
        return definition_;
    }

    bool Enemy::AllowsAimAssist() const
    {
        // active가 아니면 현재 플레이어 주변 갱신 대상이 아니므로 후보에서 제외합니다.
        // alive_가 false인 적도 사망 이펙트나 리스폰 대기 중일 수 있으므로 조준 보정을 걸지 않습니다.
        return IsActive()
            && alive_
            && definition_.aimAssistEnabled;
    }

    Vector2 Enemy::AimAssistPoint() const
    {
        // 현재는 단일 Hurtbox의 중심을 보정 지점으로 사용합니다.
        // 보스가 들어오면 이 함수 또는 별도 AimAssistTarget 컴포넌트에서 여러 후보를 만들 수 있습니다.
        const Rectangle box = Hurtbox();
        return {
            box.x + box.width * 0.5f,
            box.y + box.height * 0.5f
        };
    }

    float Enemy::AimAssistRadiusPixels() const
    {
        return definition_.aimAssistRadiusPixels;
    }

    float Enemy::AimAssistPriority() const
    {
        return definition_.aimAssistPriority;
    }

    void Enemy::ApplyCrowdSeparationPush(Vector2 push, const TileMap& tileMap)
    {
        if (!IsActive() || !alive_ || math::LengthSquared(push) <= math::VectorEpsilon)
        {
            return;
        }

        const Vector2 previousVelocity = velocity_;
        const TileCollisionMoveResult collision = tileCollision_.MoveBox(
            position_,
            push,
            HalfBodySize(),
            1.0f,
            tileMap);

        position_ = collision.position;
        velocity_ = previousVelocity;
        terrainProbe_.grounded = collision.grounded;
        terrainProbe_.hitWall = collision.hitWall;
        terrainProbe_.hitCeiling = collision.hitCeiling;
        RefreshTerrainProbe(tileMap);
    }

    void Enemy::ResetAiState()
    {
        // AI 상태 초기화:
        // - 리스폰/초기화 시 적이 과거 전투 기억이나 배회 방향을 들고 있으면
        //   스폰 직후 바로 추적하거나 이상한 방향으로 움직일 수 있으므로 모두 초기값으로 되돌립니다.
        aiState_ = EnemyAiState::Idle;
        aiStateSeconds_ = 0.0f;
        attackCooldownRemaining_ = 0.0f;
        hopCooldownRemaining_ = 0.0f;
        facingDirection_ = 1.0f;
        wanderDirection_ = 1.0f;
        wanderCycleIndex_ = 0;
        playerVisible_ = false;
        timeSinceLastSeenPlayer_ = 9999.0f;
        lastSeenPlayerPosition_ = spawnPoint_.position;
        lastKnownPlayerGroundPosition_ = spawnPoint_.position;
    }

    // UpdateAi:
    // - EnemyAiProfile 데이터와 현재 플레이어/지형 상황을 바탕으로 "이번 프레임의 의도 속도"를 정합니다.
    // - 실제 위치 확정과 타일 충돌은 UpdateMovement가 담당합니다.
    // - 따라서 이 함수는 position_을 직접 밀지 않고, velocity_와 상태 전환만 조절하는 것이 기본 규칙입니다.
    void Enemy::UpdateAi(float deltaSeconds, const EnemyAiContext& aiContext, const TileMap& tileMap)
    {
        aiStateSeconds_ += deltaSeconds;
        attackCooldownRemaining_ = std::max(0.0f, attackCooldownRemaining_ - deltaSeconds);
        hopCooldownRemaining_ = std::max(0.0f, hopCooldownRemaining_ - deltaSeconds);

        // 감지는 두 단계입니다.
        // 1. detectRange 안에 있는지 확인
        // 2. requiresLineOfSight가 켜져 있으면 타일맵 시야선이 막히지 않았는지 확인
        // 플레이어가 보이면 마지막 목격 위치를 갱신하고, 안 보이면 기억 시간을 누적합니다.
        playerVisible_ = CanSeePlayer(aiContext, tileMap);
        if (playerVisible_)
        {
            timeSinceLastSeenPlayer_ = 0.0f;
            lastSeenPlayerPosition_ = aiContext.playerWorldPosition;
        }
        else
        {
            timeSinceLastSeenPlayer_ += deltaSeconds;
        }

        if (aiContext.playerGrounded)
        {
            // 착지 예상 지점/지상 추적형 적은 플레이어가 마지막으로 땅에 있던 위치를 기준으로 압박할 수 있습니다.
            // 플레이어가 공중에 있을 때 현재 위치만 쫓으면 땅을 걷는 적이 무의미하게 위쪽을 추적하게 됩니다.
            lastKnownPlayerGroundPosition_ = aiContext.playerWorldPosition;
        }

        // leashRange 바깥까지 끌려간 적은 추적/배회를 멈추고 스폰 위치로 돌아갑니다.
        // 이렇게 해야 적이 플레이어를 따라 월드 반대편까지 이동해 배치 의도가 무너지는 일을 막습니다.
        if (ShouldReturnToSpawn() && aiState_ != EnemyAiState::Return)
        {
            ChangeAiState(EnemyAiState::Return);
        }

        switch (aiState_)
        {
        case EnemyAiState::Idle:
            // Idle:
            // - 플레이어를 보면 Alert로 전환합니다.
            // - 플레이어를 못 보고 일정 시간이 지나면 스폰 주변 Wander로 전환합니다.
            velocity_.x = 0.0f;
            if (playerVisible_)
            {
                ChangeAiState(EnemyAiState::Alert);
            }
            else if (definition_.ai.canWander && aiStateSeconds_ >= definition_.ai.idleSeconds)
            {
                BeginWander();
            }
            break;

        case EnemyAiState::Wander:
            // Wander:
            // - 활성화된 적이 플레이어를 못 보는 동안 세계가 완전히 정지해 보이지 않도록 하는 가벼운 배회입니다.
            // - 스폰 지점 주변으로만 제한하고, 벽/낭떠러지를 만나면 즉시 Idle로 돌아갑니다.
            // - 비활성 적은 EnemyPool에서 업데이트 자체를 하지 않으므로, 화면 밖 적까지 배회하지 않습니다.
            if (playerVisible_)
            {
                velocity_.x = 0.0f;
                ChangeAiState(EnemyAiState::Alert);
                break;
            }

            if (!definition_.ai.canWander || !terrainProbe_.grounded)
            {
                velocity_.x = 0.0f;
                ChangeAiState(EnemyAiState::Idle);
                break;
            }

            FaceToward(position_.x + wanderDirection_);
            velocity_.x = wanderDirection_ * definition_.moveSpeed * std::max(0.0f, definition_.ai.wanderSpeedMultiplier);

            if (std::fabs(position_.x - spawnPoint_.position.x) >= definition_.ai.wanderRadius
                || terrainProbe_.wallAhead
                || !terrainProbe_.floorAhead
                || aiStateSeconds_ >= definition_.ai.wanderSeconds)
            {
                velocity_.x = 0.0f;
                ChangeAiState(EnemyAiState::Idle);
            }
            break;

        case EnemyAiState::Alert:
            // Alert:
            // - 플레이어를 발견한 직후의 짧은 반응 시간입니다.
            // - 벽 뒤로 사라진 경우에도 pursuitMemorySeconds 안이라면 마지막 목격 위치를 바라봅니다.
            // - 기억 시간이 끝났으면 추적하지 않고 Idle로 돌아갑니다.
            velocity_.x = 0.0f;
            FaceToward(playerVisible_ ? aiContext.playerWorldPosition.x : lastSeenPlayerPosition_.x);
            if (!HasRecentPlayerSighting())
            {
                ChangeAiState(EnemyAiState::Idle);
                break;
            }

            if (aiStateSeconds_ >= ReactionSeconds(definition_.ai.reactionSpeed))
            {
                ChangeAiState(EnemyAiState::Approach);
            }
            break;

        case EnemyAiState::Approach:
        {
            // Approach:
            // - 플레이어가 보이면 현재 압박 대상 규칙을 사용합니다.
            // - 시야가 끊겼지만 기억 시간이 남아 있으면 마지막 목격 위치까지 이동합니다.
            // - 기억이 끝났으면 Return으로 빠져 스폰 배치로 돌아갈 준비를 합니다.
            if (!HasRecentPlayerSighting())
            {
                velocity_.x = 0.0f;
                ChangeAiState(EnemyAiState::Return);
                break;
            }

            const Vector2 target = ResolvePressureTarget(aiContext);
            FaceToward(target.x);

            if (CanStartAttack(target))
            {
                ChangeAiState(EnemyAiState::Telegraph);
                break;
            }

            const float speedMultiplier = definition_.ai.approachStyle == EnemyApproachStyle::Run
                ? definition_.ai.runSpeedMultiplier
                : definition_.ai.moveSpeedMultiplier;
            const float speed = definition_.moveSpeed * std::max(0.0f, speedMultiplier);
            const float xDistance = std::fabs(target.x - position_.x);

            if (xDistance <= definition_.ai.preferredRange || definition_.ai.approachStyle == EnemyApproachStyle::None)
            {
                velocity_.x = 0.0f;
            }
            else
            {
                velocity_.x = facingDirection_ * speed;
            }

            const bool usesGroundApproach =
                definition_.ai.approachStyle == EnemyApproachStyle::Walk
                || definition_.ai.approachStyle == EnemyApproachStyle::Run
                || definition_.ai.approachStyle == EnemyApproachStyle::Hop;
            if (usesGroundApproach && terrainProbe_.grounded && std::fabs(velocity_.x) > 0.0f)
            {
                // 지상형 적은 타일맵 물리로 벽을 뚫지는 않지만,
                // 벽이나 낭떠러지를 향해 계속 걷게 두면 벽 앞에서 비비거나 떨어지는 느낌이 강합니다.
                // 그래서 AI 단계에서 전진 의도를 먼저 끊습니다.
                if (terrainProbe_.wallAhead || !terrainProbe_.floorAhead)
                {
                    velocity_.x = 0.0f;
                }
            }

            if (definition_.ai.approachStyle == EnemyApproachStyle::Hop
                && terrainProbe_.grounded
                && hopCooldownRemaining_ <= 0.0f)
            {
                velocity_.y = std::max(velocity_.y, definition_.ai.hopImpulseY);
                hopCooldownRemaining_ = std::max(0.05f, definition_.ai.hopIntervalSeconds);
            }
            break;
        }

        case EnemyAiState::Telegraph:
            velocity_.x *= 0.2f;
            FaceToward(ResolvePressureTarget(aiContext).x);
            if (aiStateSeconds_ >= ResolveTelegraphSeconds())
            {
                BeginAttack(aiContext);
                ChangeAiState(EnemyAiState::Attack);
            }
            break;

        case EnemyAiState::Attack:
            if (aiStateSeconds_ >= definition_.ai.attackActiveSeconds)
            {
                ChangeAiState(EnemyAiState::Recover);
            }
            break;

        case EnemyAiState::Recover:
            velocity_.x *= 0.85f;
            if (aiStateSeconds_ >= ResolveRecoverSeconds())
            {
                ChangeAiState(EnemyAiState::Approach);
            }
            break;

        case EnemyAiState::Return:
            FaceToward(spawnPoint_.position.x);
            if (std::fabs(spawnPoint_.position.x - position_.x) <= 8.0f)
            {
                velocity_.x = 0.0f;
                ChangeAiState(playerVisible_
                    ? EnemyAiState::Alert
                    : EnemyAiState::Idle);
            }
            else
            {
                velocity_.x = facingDirection_ * definition_.moveSpeed * definition_.ai.moveSpeedMultiplier;
            }
            break;
        }
    }

    void Enemy::ChangeAiState(EnemyAiState state)
    {
        aiState_ = state;
        aiStateSeconds_ = 0.0f;
    }

    void Enemy::BeginWander()
    {
        // 배회 방향은 매번 좌/우를 번갈아 고릅니다.
        // 랜덤을 쓰면 테스트할 때 같은 상황의 결과가 계속 달라져 AI 튜닝이 어려워지므로,
        // 우선 결정적인 패턴으로 두고 나중에 필요하면 작은 랜덤 지연만 추가하는 편이 안전합니다.
        ++wanderCycleIndex_;
        wanderDirection_ = (wanderCycleIndex_ % 2 == 0) ? -1.0f : 1.0f;
        FaceToward(position_.x + wanderDirection_);
        ChangeAiState(EnemyAiState::Wander);
    }

    void Enemy::BeginAttack(const EnemyAiContext& aiContext)
    {
        const Vector2 target = ResolvePressureTarget(aiContext);
        FaceToward(target.x);

        switch (definition_.ai.attackStyle)
        {
        case EnemyAttackStyle::Lunge:
            velocity_.x = facingDirection_ * definition_.ai.lungeSpeed;
            velocity_.y = 0.0f;
            break;
        case EnemyAttackStyle::JumpPounce:
            velocity_.x = facingDirection_ * definition_.ai.jumpPounceHorizontalSpeed;
            velocity_.y = std::max(velocity_.y, definition_.ai.jumpPounceVerticalSpeed);
            break;
        case EnemyAttackStyle::Projectile:
        case EnemyAttackStyle::Area:
        case EnemyAttackStyle::Grab:
        case EnemyAttackStyle::Shield:
        case EnemyAttackStyle::Contact:
        default:
            velocity_.x = 0.0f;
            break;
        }

        attackCooldownRemaining_ = std::max(0.05f, definition_.ai.attackCooldownSeconds);
    }

    void Enemy::UpdateMovement(float deltaSeconds, const TileMap& tileMap)
    {
        // 이동 확정 단계:
        // - AI가 정한 velocity_에 중력을 더하고,
        // - TileCollisionResolver가 타일맵과의 실제 충돌을 처리합니다.
        // 이렇게 하면 적 AI가 아무리 강한 속도를 주더라도 벽/바닥/천장을 최종적으로 뚫지 않습니다.
        if (definition_.ai.approachStyle != EnemyApproachStyle::Fly
            && (!terrainProbe_.grounded || velocity_.y > 0.0f))
        {
            velocity_.y -= EnemyGroundGravity * deltaSeconds;
        }

        const TileCollisionMoveResult collision = tileCollision_.MoveBox(
            position_,
            velocity_,
            HalfBodySize(),
            deltaSeconds,
            tileMap);

        position_ = collision.position;
        velocity_ = collision.velocity;
        terrainProbe_.grounded = collision.grounded;
        terrainProbe_.hitWall = collision.hitWall;
        terrainProbe_.hitCeiling = collision.hitCeiling;

        RefreshTerrainProbe(tileMap);
    }

    void Enemy::RefreshTerrainProbe(const TileMap& tileMap)
    {
        // TerrainProbe:
        // - 이동 그 자체를 처리하는 충돌 판정이 아니라, AI 판단용 센서입니다.
        // - 앞에 벽이 있는지, 앞발 아래에 바닥이 있는지, 머리 위가 막혔는지를 미리 보고
        //   걷기/점프/배회 의도를 멈출지 결정합니다.
        const float halfSize = HalfBodySize();
        const float facing = facingDirection_ >= 0.0f ? 1.0f : -1.0f;
        const float frontX = position_.x + facing * (halfSize + EnemyWallProbeDistance);
        const float bottomY = position_.y - halfSize;

        terrainProbe_.wallAhead =
            tileMap.IsSolidAtWorld({ frontX, position_.y })
            || tileMap.IsSolidAtWorld({ frontX, position_.y - halfSize * 0.5f });
        terrainProbe_.floorAhead = tileMap.IsSolidAtWorld({ frontX, bottomY - EnemyFloorProbeDistance });
        terrainProbe_.ceilingAbove = tileMap.IsSolidAtWorld({ position_.x, position_.y + halfSize + EnemyCeilingProbeDistance });
    }

    float Enemy::BodySize() const
    {
        return std::max(1.0f, definition_.bodySize);
    }

    float Enemy::HalfBodySize() const
    {
        return BodySize() * 0.5f;
    }

    bool Enemy::CanSeePlayer(const EnemyAiContext& aiContext, const TileMap& tileMap) const
    {
        // 먼저 거리로 빠르게 거릅니다.
        // TileLineOfSight는 가벼운 타일 샘플링이지만, 모든 적이 매 프레임 무조건 호출할 필요는 없습니다.
        if (math::Distance(position_, aiContext.playerWorldPosition) > definition_.ai.detectRange)
        {
            return false;
        }

        // 일부 적은 벽 너머 감지, 소리 감지, 마법 탐지 같은 특수 규칙을 가질 수 있습니다.
        // 그런 적은 데이터에서 requiresLineOfSight를 끄면 거리 조건만으로 감지합니다.
        if (!definition_.ai.requiresLineOfSight)
        {
            return true;
        }

        // 실제 시야선 검사는 기존 TileLineOfSight를 재사용합니다.
        // 적 중심 대신 AimAssistPoint를 쓰는 이유는 현재 Hurtbox 중심이 적의 대표 위치로 이미 쓰이고 있어서,
        // 조준 보정/시야 판정 기준이 서로 크게 어긋나지 않게 하기 위해서입니다.
        return TileLineOfSight::Trace(
            tileMap,
            AimAssistPoint(),
            aiContext.playerWorldPosition).hasLineOfSight;
    }

    bool Enemy::HasRecentPlayerSighting() const
    {
        return timeSinceLastSeenPlayer_ <= std::max(0.0f, definition_.ai.pursuitMemorySeconds);
    }

    Vector2 Enemy::ResolvePressureTarget(const EnemyAiContext& aiContext) const
    {
        // 플레이어를 현재 보고 있지 않다면 마지막 목격 위치를 압박 대상으로 삼습니다.
        // 이렇게 해야 벽 하나 뒤로 숨었다고 즉시 멈추지 않고,
        // 짧은 시간 동안 "방금 본 곳까지 확인하러 가는" 행동이 됩니다.
        if (!playerVisible_)
        {
            return lastSeenPlayerPosition_;
        }

        switch (definition_.ai.pressureTarget)
        {
        case EnemyPressureTarget::LastGroundPosition:
            return lastKnownPlayerGroundPosition_;
        case EnemyPressureTarget::PredictedLandingPosition:
        {
            Vector2 target = {
                aiContext.playerWorldPosition.x + aiContext.playerWorldVelocity.x * definition_.ai.landingPredictionSeconds,
                aiContext.playerWorldPosition.y + aiContext.playerWorldVelocity.y * definition_.ai.landingPredictionSeconds
            };
            if (definition_.ai.antiAirLevel == EnemyAntiAirLevel::Ignore)
            {
                target.y = lastKnownPlayerGroundPosition_.y;
            }
            return target;
        }
        case EnemyPressureTarget::ReloadSpot:
        case EnemyPressureTarget::Platform:
        case EnemyPressureTarget::ChokePoint:
        case EnemyPressureTarget::PlayerCurrentPosition:
        default:
            return aiContext.playerWorldPosition;
        }
    }

    float Enemy::ResolveTelegraphSeconds() const
    {
        switch (definition_.ai.telegraphProfile)
        {
        case EnemyTelegraphProfile::LongStrong:
            return 0.65f;
        case EnemyTelegraphProfile::ShortWeak:
            return 0.18f;
        case EnemyTelegraphProfile::Readable:
        default:
            return 0.35f;
        }
    }

    float Enemy::ResolveRecoverSeconds() const
    {
        switch (definition_.ai.recoveryProfile)
        {
        case EnemyRecoveryProfile::LargeOpening:
            return 0.75f;
        case EnemyRecoveryProfile::SmallOpening:
            return 0.25f;
        case EnemyRecoveryProfile::Normal:
        default:
            return 0.45f;
        }
    }

    bool Enemy::CanStartAttack(Vector2 pressureTarget) const
    {
        // 공격 시작은 현재 시야가 있을 때만 허용합니다.
        // 마지막 목격 위치로 이동하는 중에 벽 너머 플레이어를 향해 돌진/점프 공격을 시작하면
        // 플레이어 입장에서는 벽 뒤 추적이 부당하게 느껴질 수 있습니다.
        if (!playerVisible_
            || attackCooldownRemaining_ > 0.0f
            || definition_.ai.attackStyle == EnemyAttackStyle::Contact)
        {
            return false;
        }

        if ((definition_.ai.attackStyle == EnemyAttackStyle::Lunge
            || definition_.ai.attackStyle == EnemyAttackStyle::JumpPounce)
            && !terrainProbe_.grounded)
        {
            return false;
        }

        return math::Distance(position_, pressureTarget) <= definition_.ai.attackRange;
    }

    bool Enemy::ShouldReturnToSpawn() const
    {
        return math::Distance(position_, spawnPoint_.position) > definition_.ai.leashRange;
    }

    void Enemy::FaceToward(float targetX)
    {
        if (targetX > position_.x + 1.0f)
        {
            facingDirection_ = 1.0f;
        }
        else if (targetX < position_.x - 1.0f)
        {
            facingDirection_ = -1.0f;
        }
    }
}

