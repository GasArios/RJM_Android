// Player.cpp
// - Player 클래스의 구현부입니다.
// - 현재는 반동 이동 물리, 바닥 착지, 무기 업데이트, 파란 사각형 그리기를 담당합니다.

#include "RecoilJumpMan/Player/Player.h"

#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/Physics/CoordinateSpace.h"
#include "RecoilJumpMan/World/TileMap.h"

#include <raylib.h>

#include <algorithm>
#include <cmath>

namespace
{
    // PlayerBodySize:
    // - 현재 프로토타입에서 파란 사각형 플레이어의 한 변 길이입니다.
    // - 기존에는 Draw 안의 지역 상수였지만, 바닥 판정과 궤적 미리보기도 같은 크기를 알아야 하므로
    //   Player::BodySize()를 통해 외부에서 읽을 수 있게 했습니다.
    constexpr float PlayerBodySize = 32.0f;
}

namespace rjm
{
    // Player 생성자:
    // - : health_(100.0f)는 멤버 초기화 리스트입니다.
    // - 생성자 본문에 들어가기 전에 health_를 최대 체력 100으로 생성합니다.
    Player::Player()
        : health_(100.0f)
    {
    }

    // Update:
    // - y-up 물리 좌표계에서 반동 이동 물리를 갱신합니다.
    // - 화면에 그릴 때만 Raylib 화면 좌표로 변환합니다.
    void Player::Update(float deltaSeconds)
    {
        UpdateMovement(deltaSeconds, nullptr, PlayerEnvironmentModifiers{});
    }

    void Player::Update(float deltaSeconds, const TileMap& tileMap)
    {
        UpdateMovement(deltaSeconds, &tileMap, PlayerEnvironmentModifiers{});
    }

    void Player::Update(
        float deltaSeconds,
        const TileMap& tileMap,
        const PlayerEnvironmentModifiers& environment)
    {
        UpdateMovement(deltaSeconds, &tileMap, environment);
    }

    void Player::UpdateMovement(
        float deltaSeconds,
        const TileMap* tileMap,
        const PlayerEnvironmentModifiers& environment)
    {
        UpdateDamageState(deltaSeconds);

        const bool wasGrounded = grounded_;
        const float positionYBeforeMove = position_.y;

        // 반동 이동의 지속 힘은 지형 처리 방식과 무관하게 한 번만 적용합니다.
        // 이렇게 해야 타일맵 경로와 fallback 바닥 경로의 조작감이 갈라지지 않습니다.
        movement_.ApplyContinuousForces(velocity_, deltaSeconds, grounded_, environment);
        if (IsBracing())
        {
            movement_.ApplyBracedGroundForces(velocity_, deltaSeconds);
        }

        if (tileMap)
        {
            const TileCollisionMoveResult collision = tileCollision_.MoveBox(
                position_,
                velocity_,
                HalfBodySize(),
                deltaSeconds,
                *tileMap);

            // resolver 결과를 Player 상태로 확정합니다.
            // grounded_는 재장전 가능 여부에도 쓰이므로, 타일 착지 결과를 그대로 반영해야 합니다.
            position_ = collision.position;
            velocity_ = collision.velocity;
            grounded_ = collision.grounded;

            if (grounded_
                && (std::fabs(environment.surfaceVelocity.x) > 0.001f
                    || std::fabs(environment.surfaceVelocity.y) > 0.001f))
            {
                const TileCollisionMoveResult surfaceMove = tileCollision_.MoveBox(
                    position_,
                    environment.surfaceVelocity,
                    HalfBodySize(),
                    deltaSeconds,
                    *tileMap);
                position_ = surfaceMove.position;
                grounded_ = surfaceMove.grounded || grounded_;
                if (surfaceMove.hitWall)
                {
                    velocity_.x = 0.0f;
                }
            }
        }
        else
        {
            // TileMap이 없는 테스트/디버그 경로에서만 평면 바닥 fallback을 사용합니다.
            position_.x += velocity_.x * deltaSeconds;
            position_.y += velocity_.y * deltaSeconds;

            if (BottomY() <= groundY_)
            {
                position_.y = groundY_ + HalfBodySize();
                if (velocity_.y < 0.0f)
                {
                    velocity_.y = 0.0f;
                }

                grounded_ = true;
            }
            else
            {
                grounded_ = false;
            }
        }

        UpdateAirborneStartY(wasGrounded, positionYBeforeMove);

        ProcessReloadBuffer(deltaSeconds);
        weapons_.Update(deltaSeconds, grounded_);
    }

    // Draw:
    // - 현재 플레이어는 파란 사각형으로 그립니다.
    void Player::Draw() const
    {
        const float size = BodySize();

        // 내부 물리 좌표를 Raylib 렌더 좌표로 변환합니다.
        // 실제 화면 좌표 변환은 BeginMode2D의 카메라가 처리합니다.
        const Vector2 renderPosition = CoordinateSpace::WorldToRender(position_);

        // Rectangle은 Raylib의 사각형 구조체입니다.
        // position_은 중심 좌표이므로, 렌더링용 왼쪽 위 좌표는 중심에서 half만큼 빼서 만듭니다.
        const float half = HalfBodySize();
        const Rectangle body = { renderPosition.x - half, renderPosition.y - half, size, size };
        const bool bracing = IsBracing();
        const bool hitFlash = hitFlashRemainingSeconds_ > 0.0f;
        const bool invulnerabilityBlink = IsInvulnerable()
            && (static_cast<int>(invulnerabilityRemainingSeconds_ * 18.0f) % 2 == 0);
        const Color bodyColor = hitFlash
            ? Color{ 170, 225, 255, 255 }
            : (invulnerabilityBlink ? Color{ 74, 160, 255, 190 } : Color{ 42, 117, 255, 255 });

        // 내부 파란색 사각형입니다.
        DrawRectangleRec(body, bodyColor);

        // 외곽선입니다.
        DrawRectangleLinesEx(
            body,
            bracing ? 3.0f : 2.0f,
            bracing ? Color{ 255, 226, 120, 255 } : Color{ 170, 210, 255, 255 });

        if (bracing)
        {
            const Rectangle braceBase = { body.x + 3.0f, body.y + body.height - 5.0f, body.width - 6.0f, 3.0f };
            DrawRectangleRec(braceBase, Color{ 255, 226, 120, 230 });
        }
    }

    Rectangle Player::Hurtbox() const
    {
        const float half = HalfBodySize();
        return {
            position_.x - half,
            position_.y - half,
            half * 2.0f,
            half * 2.0f
        };
    }

    bool Player::CanReceiveDamage() const
    {
        return IsActive()
            && !health_.IsDead()
            && !IsInvulnerable();
    }

    DamageResult Player::ApplyDamage(const Damage& damage, Vector2 hitNormal)
    {
        DamageResult result;
        result.requestedAmount = damage.amount;
        result.remainingHealth = health_.Current();

        if (!CanReceiveDamage())
        {
            return result;
        }

        result.accepted = true;
        result.appliedAmount = std::max(0.0f, damage.amount);

        health_.Damage(result.appliedAmount);
        result.remainingHealth = health_.Current();
        result.killed = health_.IsDead();

        invulnerabilityRemainingSeconds_ = invulnerabilitySeconds_;
        hitFlashRemainingSeconds_ = hitFlashSeconds_;
        ApplyDamageKnockback(damage, hitNormal);

        return result;
    }

    bool Player::IsDead() const
    {
        return health_.IsDead();
    }

    void Player::Revive(float healthRatio, float invulnerabilitySeconds)
    {
        const float safeHealthRatio = std::clamp(healthRatio, 0.01f, 1.0f);

        health_.Heal(health_.Maximum());
        health_.Damage(health_.Maximum() * (1.0f - safeHealthRatio));

        invulnerabilityRemainingSeconds_ = std::max(0.0f, invulnerabilitySeconds);
        hitFlashRemainingSeconds_ = 0.0f;
        reloadBuffer_.active = false;
        bracingRequested_ = false;
        SetActive(true);
    }

    void Player::SetPosition(Vector2 position)
    {
        position_ = position;

        // 스폰, 텔레포트, 체크포인트 복귀처럼 위치를 직접 바꾸는 순간에는
        // 이전 체공 높이를 그대로 들고 있으면 첫 발부터 과한 감쇠가 걸릴 수 있습니다.
        // 그래서 새 위치를 이번 체공/지상 기준점으로 함께 잡아둡니다.
        airborneStartY_ = position_.y;
    }

    // BodySize:
    // - 플레이어 사각형의 한 변 길이를 반환합니다.
    // - 플레이어 크기를 한곳에서 관리하면 Draw와 궤적 표시가 서로 다른 크기를 쓰는 일을 막을 수 있습니다.
    float Player::BodySize() const
    {
        return PlayerBodySize;
    }

    // HalfBodySize:
    // - 플레이어 사각형 한 변의 절반을 반환합니다.
    // - 중심 좌표에서 왼쪽 위, 바닥, 충돌 영역을 계산할 때 사용합니다.
    float Player::HalfBodySize() const
    {
        return BodySize() * 0.5f;
    }

    // BottomY:
    // - 플레이어 사각형의 바닥 y 좌표를 반환합니다.
    // - position_은 중심 좌표이고 y-up 좌표계이므로, 바닥은 중심보다 HalfBodySize만큼 아래입니다.
    float Player::BottomY() const
    {
        return position_.y - HalfBodySize();
    }

    // TryFireAt:
    // - 현재 총을 worldTarget 방향으로 발사합니다.
    // - 총이 실제로 발사되면 ShotResult의 recoil을 플레이어 속도에 적용합니다.
    ShotResult Player::TryFireAt(Vector2 worldTarget)
    {
        // 기존 단일 목표 발사는 탄환 목표와 반동 목표가 같은 가장 기본적인 경우입니다.
        return TryFireAt(worldTarget, worldTarget);
    }

    ShotResult Player::TryFireAt(Vector2 projectileWorldTarget, Vector2 recoilWorldTarget)
    {
        Gun* gun = weapons_.Current();
        if (!gun)
        {
            return {};
        }

        // 조준 방향은 목표 위치 - 플레이어 중심 위치입니다.
        // position_ 자체가 중심 좌표이므로 별도의 보정이 필요 없습니다.
        const Vector2 aimOrigin = position_;
        const Vector2 projectileAimDirection = {
            projectileWorldTarget.x - aimOrigin.x,
            projectileWorldTarget.y - aimOrigin.y
        };
        const Vector2 recoilAimDirection = {
            recoilWorldTarget.x - aimOrigin.x,
            recoilWorldTarget.y - aimOrigin.y
        };

        // ShotAim을 쓰면 탄환 방향과 반동 방향을 분리할 수 있습니다.
        // 현재 기본 무기들은 대부분 같은 방향을 쓰지만, 조준 보조 옵션이 들어오면 여기서 자연스럽게 갈라집니다.
        ShotResult shot = gun->TryFire(ShotAim{ projectileAimDirection, recoilAimDirection });
        if (shot.fired)
        {
            const Vector2 adjustedRecoil = movement_.BuildAirborneHeightAdjustedRecoilImpulse(
                shot.recoil,
                position_.y,
                airborneStartY_,
                grounded_,
                gun->Definition().altitudeDampingResistance);

            if (IsBracing())
            {
                movement_.ApplyBracedRecoilImpulse(velocity_, adjustedRecoil);
            }
            else
            {
                movement_.ApplyRecoilImpulse(velocity_, adjustedRecoil);
            }
        }

        return shot;
    }

    ReloadBatchResult Player::RequestReload(ReloadIntent intent)
    {
        const ReloadBatchResult result = TryReload(intent);
        if (result.StartedAny())
        {
            reloadBuffer_.active = false;
            return result;
        }

        if (!grounded_ && result.accepted)
        {
            StoreReloadBuffer(intent);
        }

        return result;
    }

    // TryReload:
    // - 장착된 모든 총의 재장전을 즉시 시도합니다.
    // - Player는 자신이 지상에 있는지만 알고, "R키 재장전인지 빈 탄창 클릭인지"는 인벤토리에 intent로 넘깁니다.
    ReloadBatchResult Player::TryReload(ReloadIntent intent)
    {
        return weapons_.TryReloadAll(grounded_, intent);
    }

    void Player::SetBracing(bool bracing)
    {
        bracingRequested_ = bracing;
    }

    // IsGrounded:
    // - 착지 상태를 반환합니다.
    bool Player::IsGrounded() const
    {
        return grounded_;
    }

    float Player::AirborneStartY() const
    {
        return airborneStartY_;
    }

    bool Player::IsBracing() const
    {
        return bracingRequested_ && grounded_;
    }

    bool Player::IsInvulnerable() const
    {
        return invulnerabilityRemainingSeconds_ > 0.0f;
    }

    // SetGroundY:
    // - 임시 바닥 높이를 설정합니다.
    void Player::SetGroundY(float groundY)
    {
        groundY_ = groundY;
    }

    // Weapons:
    // - 무기 인벤토리를 수정 가능한 참조로 반환합니다.
    WeaponInventory& Player::Weapons()
    {
        return weapons_;
    }

    // const 버전 Weapons:
    // - 읽기 전용 상황에서 사용됩니다.
    const WeaponInventory& Player::Weapons() const
    {
        return weapons_;
    }

    // Movement:
    // - 플레이어가 실제로 사용하는 반동 이동 컨트롤러를 읽기 전용으로 반환합니다.
    // - 궤적 미리보기는 이 컨트롤러를 받아 같은 ApplyRecoilImpulse / ApplyContinuousForces를 사용합니다.
    // - const 참조를 반환하므로 외부 시스템이 플레이어의 물리 튜닝을 마음대로 바꿀 수는 없습니다.
    const RecoilMovementController& Player::Movement() const
    {
        return movement_;
    }

    // HitPoints:
    // - 체력을 수정 가능한 참조로 반환합니다.
    Health& Player::HitPoints()
    {
        return health_;
    }

    // const 버전 HitPoints:
    // - HUD처럼 체력을 읽기만 하는 코드에서 사용됩니다.
    const Health& Player::HitPoints() const
    {
        return health_;
    }

    void Player::StoreReloadBuffer(ReloadIntent intent)
    {
        reloadBuffer_.active = true;
        reloadBuffer_.intent = intent;
        reloadBuffer_.remainingSeconds = reloadBufferSeconds_;
    }

    void Player::ProcessReloadBuffer(float deltaSeconds)
    {
        if (!reloadBuffer_.active)
        {
            return;
        }

        if (grounded_)
        {
            TryReload(reloadBuffer_.intent);
            reloadBuffer_.active = false;
            return;
        }

        if (deltaSeconds <= 0.0f)
        {
            return;
        }

        reloadBuffer_.remainingSeconds -= deltaSeconds;
        if (reloadBuffer_.remainingSeconds <= 0.0f)
        {
            reloadBuffer_.active = false;
        }
    }

    void Player::UpdateDamageState(float deltaSeconds)
    {
        if (deltaSeconds <= 0.0f)
        {
            return;
        }

        invulnerabilityRemainingSeconds_ = std::max(0.0f, invulnerabilityRemainingSeconds_ - deltaSeconds);
        hitFlashRemainingSeconds_ = std::max(0.0f, hitFlashRemainingSeconds_ - deltaSeconds);
    }

    void Player::ApplyDamageKnockback(const Damage& damage, Vector2 hitNormal)
    {
        const Vector2 normal = math::NormalizeOr(hitNormal, { 0.0f, 1.0f });
        const float impact = std::max(0.0f, damage.impactForce);

        if (std::fabs(normal.x) > math::VectorEpsilon)
        {
            const float horizontalSpeed = std::min(
                hitKnockbackHorizontalSpeed_,
                std::max(hitKnockbackHorizontalSpeed_ * 0.45f, impact));
            velocity_.x += normal.x * horizontalSpeed;
        }

        if (normal.y >= -0.35f)
        {
            velocity_.y = std::max(
                velocity_.y,
                hitKnockbackUpwardSpeed_ + std::max(0.0f, normal.y) * impact * 0.25f);
        }
        else
        {
            velocity_.y += normal.y * impact * 0.35f;
        }
    }

    void Player::UpdateAirborneStartY(bool wasGrounded, float positionYBeforeMove)
    {
        if (grounded_)
        {
            // 착지 중에는 현재 중심 y를 계속 기준점으로 갱신합니다.
            // 이렇게 해두면 다음 발사로 떠오를 때 "마지막으로 밟고 있던 높이"가 자연스럽게 시작점이 됩니다.
            airborneStartY_ = position_.y;
            return;
        }

        if (wasGrounded)
        {
            // 바로 이번 프레임에 지상에서 공중으로 바뀐 경우입니다.
            // 이동 후 위치를 쓰면 이미 살짝 떠오른 높이가 기준점이 되어 감쇠가 늦게 걸리므로,
            // 이동 직전의 지상 위치를 체공 시작 높이로 기록합니다.
            airborneStartY_ = positionYBeforeMove;
        }
    }
}

