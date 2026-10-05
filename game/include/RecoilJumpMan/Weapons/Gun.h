#pragma once

// Gun.h
// - 총 한 자루의 상태와 발사/재장전 기능을 선언합니다.
// - WeaponDefinition이 "총의 기본 설계도"라면, Gun은 "현재 탄약과 쿨타임을 가진 실제 총"입니다.

#include "RecoilJumpMan/Combat/Damage.h"
#include "RecoilJumpMan/Data/WeaponDefinition.h"

#include <raylib.h>

#include <vector>

namespace rjm
{
    // ReloadIntent:
    // - 재장전 요청이 "왜" 들어왔는지 구분합니다.
    // - InputState는 R키/좌클릭 같은 입력 사실만 알고,
    //   실제로 그 입력이 어떤 재장전 의도인지는 GameplayScene 쪽 게임 로직이 판단합니다.
    enum class ReloadIntent
    {
        // Manual:
        // - R 키로 직접 재장전하는 경우입니다.
        // - 탄창이 가득 차 있지만 않다면, 남은 탄이 1발 이상이어도 재장전을 허용합니다.
        Manual,

        // EmptyMagazineOnly:
        // - 좌클릭을 눌렀는데 탄창이 0발인 경우입니다.
        // - 탄이 조금이라도 남아 있다면 좌클릭은 재장전이 아니라 발사 시도로 처리해야 합니다.
        EmptyMagazineOnly
    };

    // ShotProjectile:
    // - 한 번의 발사에서 실제로 생성할 투사체 하나의 데이터입니다.
    // - 일반 총은 1개만 만들고, 샷건은 여러 펠릿을 이 배열에 담습니다.
    struct ShotProjectile
    {
        Damage damage;
        Vector2 velocity = { 0.0f, 0.0f };
        float lifetimeSeconds = 2.0f;
        float hitRadius = 4.0f;
    };

    // ShotResult:
    // - 총을 발사하려고 했을 때 결과를 담는 구조체입니다.
    // - 발사 성공 여부, 데미지, 반동, 탄환 속도를 함께 반환합니다.
    // - Gun은 카메라나 시간 시스템을 직접 만지지 않고, 필요한 결과만 이 구조체에 담아 돌려줍니다.
    // - 이렇게 해야 총기 로직이 "전투 수치 계산"에 집중하고, 연출은 GameplayScene/FeedbackSystem에서 처리할 수 있습니다.
    struct ShotResult
    {
        // 실제로 발사되었으면 true입니다.
        bool fired = false;

        // 발사된 탄환이 줄 피해 정보입니다.
        Damage damage;

        // 플레이어에게 적용할 반동 벡터입니다.
        Vector2 recoil = { 0.0f, 0.0f };

        // 탄환이 날아갈 대표 속도 벡터입니다.
        // PelletSpread에서는 중앙 조준 방향의 속도를 담아 기존 미리보기/디버그 경로가 한쪽 끝 펠릿으로 치우치지 않게 합니다.
        Vector2 projectileVelocity = { 0.0f, 0.0f };

        // 한 번의 발사에서 생성할 투사체 목록입니다.
        // 일반 총은 1개, PelletSpread 샷건은 여러 개가 들어갑니다.
        std::vector<ShotProjectile> projectiles;

        // 발사 성공 순간 발생시킬 손맛/연출 데이터입니다.
        // GameplayScene이 이 값을 GameFeedbackSystem에 넘겨 역경직과 카메라 흔들림을 발생시킵니다.
        WeaponFeedbackProfile feedback;
    };

    // ShotAim:
    // - 한 번의 발사에서 "탄환이 나갈 방향"과 "플레이어가 반동을 받을 방향"을 분리해서 담습니다.
    // - 평소에는 두 값이 같지만, 조준 보조/스킬/특수탄이 들어오면 둘을 다르게 줄 수 있습니다.
    // - 예: 탄은 적에게 보정하지만 반동은 원래 마우스 방향을 유지하는 접근성 옵션.
    struct ShotAim
    {
        // projectileDirection:
        // - 탄환 속도를 계산할 방향입니다.
        Vector2 projectileDirection = { 1.0f, 0.0f };

        // recoilDirection:
        // - 반동을 계산할 기준 방향입니다.
        // - 실제 recoil 벡터는 이 방향의 반대 방향으로 만들어집니다.
        Vector2 recoilDirection = { 1.0f, 0.0f };
    };

    class Gun
    {
    public:
        // 기본 생성자:
        // - std::optional<Gun> 같은 컨테이너에서 빈 값 처리를 쉽게 하기 위해 둡니다.
        Gun() = default;

        // explicit 생성자:
        // - WeaponDefinition을 받아 실제 Gun 객체를 만듭니다.
        explicit Gun(WeaponDefinition definition);

        // Update:
        // - 발사 쿨타임과 재장전 시간을 갱신합니다.
        // - grounded가 true일 때만 재장전 진행을 허용합니다.
        void Update(float deltaSeconds, bool grounded);

        // TryFire:
        // - 조준 방향으로 발사를 시도합니다.
        // - 탄이 없거나 쿨타임/재장전 중이면 fired=false인 결과를 반환합니다.
        ShotResult TryFire(Vector2 aimDirection);

        // TryFire:
        // - 탄환 방향과 반동 방향을 분리해서 발사를 시도합니다.
        // - 조준 보조처럼 "맞추는 방향"과 "이동하는 방향"이 달라질 수 있는 기능에서 사용합니다.
        ShotResult TryFire(ShotAim aim);

        // PreviewShot:
        // - 탄약, 쿨타임, 재장전 상태를 바꾸지 않고 발사 결과만 계산합니다.
        // - 궤적 미리보기처럼 "지금 쏘면 어떤 반동이 나오는가"를 보여줄 때 사용합니다.
        // - requireReadyToFire가 true면 TryFire와 같은 발사 가능 조건을 검사합니다.
        // - false로 넘기면 탄이 없거나 쿨타임 중이어도 데이터상 반동 경로를 계산할 수 있습니다.
        //   튜토리얼, 도감, 무기 비교 UI 같은 곳에서 유용할 수 있습니다.
        ShotResult PreviewShot(Vector2 aimDirection, bool requireReadyToFire = true) const;

        // PreviewShot:
        // - ShotAim 버전의 미리보기입니다.
        // - 실제 발사와 같은 탄/반동 분리 계산을 궤적 미리보기에서도 사용할 수 있게 합니다.
        ShotResult PreviewShot(ShotAim aim, bool requireReadyToFire = true) const;

        // CanFire:
        // - 현재 프레임에 실제 발사가 가능한지 확인합니다.
        // - 탄약, 발사 쿨타임, 재장전 상태만 검사합니다.
        // - 입력 focus나 플레이어 상태 같은 더 큰 게임 규칙은 GameplayScene/Player 쪽에서 판단합니다.
        bool CanFire() const;

        // CycleFireMode:
        // - 현재 무기가 지원하는 다음 발사 모드로 전환합니다.
        // - 실제로 모드가 바뀌면 true, 바꿀 수 있는 다른 모드가 없으면 false를 반환합니다.
        bool CycleFireMode();

        // TryReload:
        // - 지상에 있을 때 재장전을 시작합니다.
        // - intent를 통해 R키 재장전과 빈 탄창 좌클릭 재장전을 구분합니다.
        // - 재장전이 실제로 시작되면 true, 조건이 맞지 않으면 false를 반환합니다.
        bool TryReload(bool grounded, ReloadIntent intent);

        // StartReload:
        // - 외부에서 계산한 재장전 시간으로 이 총의 재장전 상태를 시작합니다.
        // - WeaponInventory의 세트 재장전처럼 여러 총을 같은 시간으로 묶을 때 사용합니다.
        void StartReload(float reloadSeconds);

        // NeedsReload:
        // - 탄창이 가득 차 있지 않으면 true입니다.
        bool NeedsReload() const;

        // FillMagazine:
        // - 탄창을 즉시 가득 채웁니다.
        void FillMagazine();

        // Definition:
        // - 이 총의 기본 능력치 데이터를 반환합니다.
        const WeaponDefinition& Definition() const;

        // FireMode:
        // - 현재 선택된 발사 모드를 반환합니다.
        FireControlMode FireMode() const;

        // SupportsFireMode:
        // - 이 총이 특정 발사 모드를 지원하는지 확인합니다.
        bool SupportsFireMode(FireControlMode mode) const;

        // AmmoInMagazine:
        // - 현재 탄창에 남은 탄 수를 반환합니다.
        int AmmoInMagazine() const;

        // IsReloading:
        // - 현재 재장전 중인지 반환합니다.
        bool IsReloading() const;

    private:
        // 총의 기본 데이터입니다.
        WeaponDefinition definition_;

        // 현재 발사 모드입니다.
        // WeaponDefinition의 defaultFireMode로 시작하지만, 플레이어가 B키로 바꿀 수 있습니다.
        FireControlMode fireMode_ = FireControlMode::SemiAuto;

        // 현재 탄창에 남은 탄 수입니다.
        int ammoInMagazine_ = 0;

        // 다음 발사를 하기 전까지 남은 시간입니다.
        float cooldownRemaining_ = 0.0f;

        // 재장전 완료까지 남은 시간입니다.
        float reloadRemaining_ = 0.0f;

        // ResolveSupportedFireMode:
        // - 데이터가 잘못되어 지원하지 않는 모드가 기본값으로 들어와도 안전한 모드로 보정합니다.
        FireControlMode ResolveSupportedFireMode(FireControlMode requestedMode) const;

        // BuildShot:
        // - 총기 데이터를 발사 결과로 바꾸는 순수 계산입니다.
        // - TryFire와 PreviewShot이 같은 계산을 쓰도록 묶어둡니다.
        // - 이 함수는 ammoInMagazine_, cooldownRemaining_, reloadRemaining_을 바꾸지 않습니다.
        // - 실제 상태 변경은 TryFire가 담당하고, 계산 재사용은 이 함수가 담당합니다.
        ShotResult BuildShot(ShotAim aim) const;
    };
}

