#pragma once

// Player.h
// - 플레이어 캐릭터 클래스의 선언부입니다.
// - 이 게임의 주인공인 파란 사각형의 상태와 기능을 담습니다.

#include "RecoilJumpMan/Combat/Health.h"
#include "RecoilJumpMan/Combat/Damage.h"
#include "RecoilJumpMan/Entity/Entity.h"
#include "RecoilJumpMan/Physics/RecoilMovementController.h"
#include "RecoilJumpMan/Physics/TileCollisionResolver.h"
#include "RecoilJumpMan/Weapons/Gun.h"
#include "RecoilJumpMan/Weapons/WeaponInventory.h"

namespace rjm
{
    class TileMap;

    // Player는 Entity를 public 상속합니다.
    // 즉 Player는 Entity의 일종이며, 위치/속도/활성 상태를 물려받습니다.
    class Player final : public Entity
    {
    public:
        // 생성자:
        // - 플레이어 체력 같은 기본값을 설정합니다.
        Player();

        // Update:
        // - 매 프레임 플레이어 물리 상태를 갱신합니다.
        void Update(float deltaSeconds) override;

        // Update:
        // - TileMap 기반 지형 충돌을 사용해 플레이어 물리 상태를 갱신합니다.
        void Update(float deltaSeconds, const TileMap& tileMap);

        // Update:
        // - TileMap 충돌과 환경 modifier를 함께 사용해 플레이어 물리 상태를 갱신합니다.
        void Update(
            float deltaSeconds,
            const TileMap& tileMap,
            const PlayerEnvironmentModifiers& environment);

        // Draw:
        // - 플레이어를 화면에 그립니다.
        void Draw() const override;

        // Hurtbox:
        // - 적 접촉, 함정, 보스 공격이 플레이어를 맞혔는지 검사할 사각형입니다.
        Rectangle Hurtbox() const;

        // CanReceiveDamage:
        // - 현재 무적 시간이나 사망 상태 때문에 피해를 무시해야 하는지 확인합니다.
        bool CanReceiveDamage() const;

        // ApplyDamage:
        // - 플레이어에게 피해를 적용하고, 무적 시간/피격 플래시/넉백을 시작합니다.
        DamageResult ApplyDamage(const Damage& damage, Vector2 hitNormal);

        // IsDead:
        // - 현재 임시 육체가 전투 불능 상태인지 반환합니다.
        bool IsDead() const;

        // Revive:
        // - 부활 앵커에서 임시 육체를 재구성한 뒤 체력과 짧은 무적 시간을 부여합니다.
        void Revive(float healthRatio = 1.0f, float invulnerabilitySeconds = 1.25f);

        // SetPosition:
        // - 플레이어 위치를 직접 설정합니다.
        // - Entity의 SetPosition과 같은 역할을 하지만, 플레이어 전용 체공 기준점도 함께 초기화합니다.
        void SetPosition(Vector2 position);

        // BodySize:
        // - 현재 파란 사각형 플레이어의 한 변 길이를 반환합니다.
        // - Draw, 바닥 판정, 궤적 미리보기처럼 "플레이어가 화면에서 얼마나 큰가"를 알아야 하는 곳에서 사용합니다.
        float BodySize() const;

        // HalfBodySize:
        // - 플레이어 사각형 한 변 길이의 절반을 반환합니다.
        // - 중심 좌표에서 왼쪽 위/오른쪽 아래/바닥 좌표를 계산할 때 반복해서 사용합니다.
        float HalfBodySize() const;

        // BottomY:
        // - 플레이어 사각형의 바닥 월드 y 좌표를 반환합니다.
        // - Position().y는 중심 좌표이므로, 바닥은 position_.y - HalfBodySize()입니다.
        float BottomY() const;

        // TryFireAt:
        // - worldTarget 방향으로 현재 총을 발사하려고 시도합니다.
        // - 성공하면 총의 반동을 플레이어 속도에 즉시 적용하고 ShotResult를 반환합니다.
        ShotResult TryFireAt(Vector2 worldTarget);

        // TryFireAt:
        // - 탄환 목표와 반동 목표를 분리해서 현재 총을 발사하려고 시도합니다.
        // - 조준 보조가 탄만 적에게 보정하고 반동은 원래 마우스 방향으로 유지하는 경우에 사용합니다.
        ShotResult TryFireAt(Vector2 projectileWorldTarget, Vector2 recoilWorldTarget);

        // RequestReload:
        // - 재장전 입력 의도를 처리합니다.
        // - 지금 가능하면 즉시 재장전하고, 공중이라서 실패하면 짧은 입력 버퍼에 저장합니다.
        ReloadBatchResult RequestReload(ReloadIntent intent);

        // TryReload:
        // - 장착된 모든 총의 재장전을 즉시 시도합니다.
        // - 입력 버퍼 없이 현재 grounded_ 상태만 보고 판단합니다.
        ReloadBatchResult TryReload(ReloadIntent intent);

        // SetBracing:
        // - 버티기 키를 누르고 있는지 플레이어 상태에 기록합니다.
        // - 실제 버티기 효과는 지상에 있을 때만 적용됩니다.
        void SetBracing(bool bracing);

        // IsGrounded:
        // - 플레이어가 땅에 닿아 있는지 반환합니다.
        bool IsGrounded() const;

        // AirborneStartY:
        // - 이번 체공을 시작했을 때의 플레이어 중심 y 좌표를 반환합니다.
        // - 위쪽 반동 효율 감소와 궤적 미리보기가 같은 기준 높이를 쓰기 위해 공개합니다.
        float AirborneStartY() const;

        // IsBracing:
        // - 버티기 입력 중이고 지상에 있을 때 true입니다.
        bool IsBracing() const;

        // IsInvulnerable:
        // - 피격 후 짧은 무적 시간이 남아 있는지 반환합니다.
        bool IsInvulnerable() const;

        // SetGroundY:
        // - TileMap이 없는 테스트/fallback 이동 경로에서 사용할 평면 지면 높이를 설정합니다.
        void SetGroundY(float groundY);

        // Weapons:
        // - 플레이어의 무기 인벤토리를 수정 가능한 참조로 반환합니다.
        WeaponInventory& Weapons();

        // const 버전 Weapons:
        // - const Player에서도 무기 상태를 읽을 수 있게 합니다.
        const WeaponInventory& Weapons() const;

        // Movement:
        // - 현재 플레이어가 사용하는 반동 이동 컨트롤러를 읽기 전용으로 반환합니다.
        // - 궤적 미리보기는 이 컨트롤러를 재사용해 실제 이동과 같은 튜닝값으로 시뮬레이션합니다.
        const RecoilMovementController& Movement() const;

        // HitPoints:
        // - 플레이어 체력을 수정 가능한 참조로 반환합니다.
        Health& HitPoints();

        // const 버전 HitPoints:
        // - const Player에서도 체력 값을 읽을 수 있게 합니다.
        const Health& HitPoints() const;

    private:
        struct ReloadInputBuffer
        {
            bool active = false;
            ReloadIntent intent = ReloadIntent::Manual;
            float remainingSeconds = 0.0f;
        };

        // StoreReloadBuffer:
        // - 공중에서 들어온 재장전 의도를 짧은 시간 동안 보관합니다.
        void StoreReloadBuffer(ReloadIntent intent);

        // ProcessReloadBuffer:
        // - 착지 직후 버퍼에 저장된 재장전 의도를 한 번 실행합니다.
        void ProcessReloadBuffer(float deltaSeconds);

        // UpdateDamageState:
        // - 무적 시간과 피격 플래시 타이머를 갱신합니다.
        void UpdateDamageState(float deltaSeconds);

        // ApplyDamageKnockback:
        // - 피격 방향과 충격력을 플레이어 속도에 반영합니다.
        void ApplyDamageKnockback(const Damage& damage, Vector2 hitNormal);

        // UpdateMovement:
        // - 지속 힘, 지형 충돌/fallback 바닥, 체공 기준점, 재장전 버퍼를 한 경로에서 처리합니다.
        void UpdateMovement(
            float deltaSeconds,
            const TileMap* tileMap,
            const PlayerEnvironmentModifiers& environment);

        // UpdateAirborneStartY:
        // - 지상에서 공중으로 전환되는 순간의 y 좌표를 기록하고, 착지하면 현재 지상 y로 갱신합니다.
        void UpdateAirborneStartY(bool wasGrounded, float positionYBeforeMove);

        // 플레이어가 장착한 총 3개를 관리합니다.
        WeaponInventory weapons_;

        // 플레이어 체력입니다.
        Health health_;

        // 반동 이동 전용 물리 컨트롤러입니다.
        RecoilMovementController movement_;

        // TileMap과의 AABB 충돌을 처리합니다.
        TileCollisionResolver tileCollision_;

        // TileMap이 없는 테스트/fallback 경로에서만 쓰는 평면 지면 높이입니다.
        // 게임 내부 물리 좌표 기준이며, y+는 위쪽입니다.
        float groundY_ = 180.0f;

        // 플레이어가 땅에 닿아 있는지 나타냅니다.
        bool grounded_ = false;

        // 이번 체공을 시작했을 때의 플레이어 중심 y 좌표입니다.
        // y-up 좌표계이므로 position_.y - airborneStartY_가 클수록 더 높은 곳까지 올라간 상태입니다.
        float airborneStartY_ = 0.0f;

        // 버티기 키를 누르고 있는지 나타냅니다.
        // 공중에서는 입력을 저장만 하고 실제 반동 흡수 효과는 적용하지 않습니다.
        bool bracingRequested_ = false;

        // 착지 직전에 누른 재장전 입력을 보관하는 버퍼입니다.
        ReloadInputBuffer reloadBuffer_;

        // 재장전 입력을 착지 후 자동 처리할 수 있는 최대 시간입니다.
        float reloadBufferSeconds_ = 1.0f;

        // 피격 후 연속 피해를 막는 짧은 무적 시간입니다.
        float invulnerabilityRemainingSeconds_ = 0.0f;
        float invulnerabilitySeconds_ = 0.75f;

        // 피격 순간 색을 밝게 바꾸는 짧은 표시 시간입니다.
        float hitFlashRemainingSeconds_ = 0.0f;
        float hitFlashSeconds_ = 0.12f;

        // 적 접촉 피격 시 적용할 넉백 튜닝입니다.
        float hitKnockbackHorizontalSpeed_ = 520.0f;
        float hitKnockbackUpwardSpeed_ = 380.0f;
    };
}

