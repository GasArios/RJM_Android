// Gun.cpp
// - Gun 클래스의 구현부입니다.
// - 총기 발사, 탄약 감소, 반동 계산, 재장전 진행을 담당합니다.

#include "RecoilJumpMan/Weapons/Gun.h"

#include "RecoilJumpMan/Math/VectorMath.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace
{
    // RotateDegrees:
    // - 이미 정규화된 방향 벡터를 지정 각도만큼 회전합니다.
    // - 샷건 펠릿처럼 "중앙 조준 방향을 기준으로 좌우로 퍼지는 탄"을 만들 때 사용합니다.
    // - 이 함수는 길이를 보존하지만, 호출부에서 다시 Normalize해 부동소수 오차를 정리합니다.
    Vector2 RotateDegrees(Vector2 direction, float degrees)
    {
        const float radians = degrees * rjm::math::Pi / 180.0f;
        const float cosine = std::cos(radians);
        const float sine = std::sin(radians);
        return {
            direction.x * cosine - direction.y * sine,
            direction.x * sine + direction.y * cosine
        };
    }

    // ResolveProjectileLifetime:
    // - 무기 데이터에 projectileRange가 있으면 "사거리 / 탄속"으로 투사체 수명을 계산합니다.
    // - 이렇게 하면 샷건은 탄속이 빨라도 짧은 시간 뒤 사라져 짧은 사거리 무기가 되고,
    //   리볼버/볼트액션은 긴 사거리를 데이터로 직접 표현할 수 있습니다.
    // - projectileRange가 비어 있는 옛 데이터는 projectileLifetimeSeconds를 그대로 사용합니다.
    float ResolveProjectileLifetime(const rjm::WeaponDefinition& definition)
    {
        if (definition.projectileRange > 0.0f && definition.bulletSpeed > 0.0f)
        {
            return definition.projectileRange / definition.bulletSpeed;
        }

        return std::max(0.01f, definition.projectileLifetimeSeconds);
    }
}

namespace rjm
{
    // Gun 생성자:
    // - WeaponDefinition을 받아 definition_에 저장합니다.
    // - 탄창은 처음에 가득 찬 상태로 시작합니다.
    Gun::Gun(WeaponDefinition definition)
        : definition_(std::move(definition)), ammoInMagazine_(definition_.magazineSize)
    {
        fireMode_ = ResolveSupportedFireMode(definition_.defaultFireMode);
    }

    // Update:
    // - 쿨타임과 재장전 시간을 프레임 시간만큼 줄입니다.
    void Gun::Update(float deltaSeconds, bool grounded)
    {
        // 발사 쿨타임이 남아 있다면 시간을 줄입니다.
        if (cooldownRemaining_ > 0.0f)
        {
            cooldownRemaining_ -= deltaSeconds;
        }

        // 재장전은 grounded가 true, 즉 지상에 있을 때만 진행합니다.
        if (reloadRemaining_ > 0.0f && grounded)
        {
            reloadRemaining_ -= deltaSeconds;

            // 재장전 시간이 끝나면 탄창을 가득 채웁니다.
            if (reloadRemaining_ <= 0.0f)
            {
                FillMagazine();
            }
        }
    }

    // TryFire:
    // - 실제 발사를 시도합니다.
    // - 성공하면 탄약을 1발 줄이고, 다음 발사를 막기 위한 쿨타임을 설정한 뒤 ShotResult를 반환합니다.
    // - 실패하면 fired=false인 빈 ShotResult를 반환합니다.
    ShotResult Gun::TryFire(Vector2 aimDirection)
    {
        // 기존 호출부는 하나의 조준 방향만 넘깁니다.
        // 내부에서는 탄 방향과 반동 방향을 같은 값으로 묶어 ShotAim 버전으로 위임합니다.
        return TryFire(ShotAim{ aimDirection, aimDirection });
    }

    ShotResult Gun::TryFire(ShotAim aim)
    {
        // CanFire:
        // - 탄약이 남아 있는지
        // - 쿨타임이 끝났는지
        // - 재장전 중이 아닌지
        // 위 세 조건을 한곳에서 검사합니다.
        if (!CanFire())
        {
            return {};
        }

        // 탄을 1발 소모합니다.
        --ammoInMagazine_;

        // 다음 발사를 막기 위해 쿨타임을 설정합니다.
        cooldownRemaining_ = definition_.fireCooldownSeconds;

        return BuildShot(aim);
    }

    // PreviewShot:
    // - "지금 이 방향으로 쏘면 어떤 ShotResult가 나오는가"만 계산합니다.
    // - 탄약을 줄이지 않고, 쿨타임도 걸지 않고, 재장전 상태도 바꾸지 않습니다.
    // - 궤적 미리보기는 이 함수를 사용해 실제 게임 상태를 건드리지 않은 채 반동 벡터만 얻습니다.
    ShotResult Gun::PreviewShot(Vector2 aimDirection, bool requireReadyToFire) const
    {
        // 단일 방향 미리보기는 탄 방향과 반동 방향을 동일하게 취급합니다.
        return PreviewShot(ShotAim{ aimDirection, aimDirection }, requireReadyToFire);
    }

    ShotResult Gun::PreviewShot(ShotAim aim, bool requireReadyToFire) const
    {
        // requireReadyToFire가 true면 실제 발사 가능 조건과 같은 조건을 적용합니다.
        // false라면 "현재는 못 쏘지만 데이터상 반동은 이렇다"를 계산할 수 있습니다.
        if (requireReadyToFire && !CanFire())
        {
            return {};
        }

        return BuildShot(aim);
    }

    // CanFire:
    // - 현재 총 자체의 상태만 보고 발사 가능 여부를 반환합니다.
    // - 마우스를 눌렀는지, UI가 열려 있는지, 플레이어가 행동 불가 상태인지는 여기서 판단하지 않습니다.
    // - 그런 판단까지 Gun이 알게 되면 총기 로직이 UI/플레이어 상태에 과하게 의존하게 됩니다.
    bool Gun::CanFire() const
    {
        // 세 조건이 모두 만족되어야 발사할 수 있습니다.
        // - cooldownRemaining_ <= 0: 이전 발사의 쿨타임이 끝남
        // - reloadRemaining_ <= 0: 재장전 중이 아님
        // - ammoInMagazine_ > 0: 탄창에 탄이 남아 있음
        return cooldownRemaining_ <= 0.0f
            && reloadRemaining_ <= 0.0f
            && ammoInMagazine_ > 0;
    }

    // BuildShot:
    // - WeaponDefinition의 데이터와 ShotAim을 ShotResult로 변환합니다.
    // - 이 함수는 "계산만" 합니다.
    // - 그래서 실제 발사 TryFire와 예측 발사 PreviewShot이 같은 반동/탄속/데미지 계산을 공유할 수 있습니다.
    ShotResult Gun::BuildShot(ShotAim aim) const
    {
        // 탄환 방향과 반동 방향을 각각 길이 1짜리 방향 벡터로 바꿉니다.
        // 대부분의 무기는 두 방향이 같지만, 조준 보조/스킬/특수탄은 둘을 다르게 줄 수 있습니다.
        const Vector2 projectileDirection = math::NormalizeOrRight(aim.projectileDirection);
        const Vector2 recoilDirection = math::NormalizeOrRight(aim.recoilDirection);

        ShotResult result;
        result.fired = true;

        // 이 탄환이 줄 피해 정보를 만듭니다.
        result.damage = Damage{ definition_.damage, DamageType::Physical, definition_.impactForce };

        // 반동은 recoilDirection의 반대 방향입니다.
        // 보통은 탄환 방향과 같으므로 오른쪽으로 쏘면 플레이어는 왼쪽으로 밀립니다.
        // 조준 보조 옵션에 따라 탄은 적에게 보정하고 반동은 원래 마우스 방향을 유지할 수도 있습니다.
        result.recoil = {
            -recoilDirection.x * definition_.recoilForce,
            -recoilDirection.y * definition_.recoilForce
        };

        // 탄환 속도는 projectileDirection * 탄속입니다.
        result.projectileVelocity = {
            projectileDirection.x * definition_.bulletSpeed,
            projectileDirection.y * definition_.bulletSpeed
        };

        // 아래부터는 실제로 생성할 투사체 목록을 만듭니다.
        // - SingleProjectile/AutoProjectile: 1개 생성
        // - PelletSpread: projectileCount개 생성
        // 반동과 탄약 소모는 TryFire에서 이미 "발사 1회" 기준으로 처리했기 때문에,
        // 여기서는 탄환 개수만 늘려도 샷건이 펠릿 수만큼 반동을 여러 번 받지 않습니다.
        const float projectileLifetimeSeconds = ResolveProjectileLifetime(definition_);
        const float projectileHitRadius = std::max(0.5f, definition_.projectileHitRadius);
        const int projectileCount = std::max(1, definition_.projectileCount);
        const bool spreadShot = definition_.attackPattern == WeaponAttackPattern::PelletSpread && projectileCount > 1;
        const float totalSpreadDegrees = spreadShot ? std::max(0.0f, definition_.spreadDegrees) : 0.0f;

        result.projectiles.reserve(static_cast<std::size_t>(projectileCount));
        for (int projectileIndex = 0; projectileIndex < projectileCount; ++projectileIndex)
        {
            float angleOffsetDegrees = 0.0f;
            if (spreadShot && projectileCount > 1)
            {
                // 펠릿은 랜덤이 아니라 균등 분포로 배치합니다.
                // 프로토타입 단계에서는 같은 조준/같은 무기라면 매번 같은 결과가 나와야
                // 데미지, 반동, 사거리 튜닝을 원인 추적하기 쉽습니다.
                const float normalizedIndex = static_cast<float>(projectileIndex) / static_cast<float>(projectileCount - 1);
                angleOffsetDegrees = (normalizedIndex - 0.5f) * totalSpreadDegrees;
            }

            // PelletSpread가 아닌 무기는 angleOffsetDegrees가 0이라 중앙 방향 한 발만 생성됩니다.
            // 즉 새로운 공격 패턴이 추가되기 전까지 기존 무기는 이 경로를 그대로 재사용합니다.
            const Vector2 pelletDirection = math::NormalizeOrRight(RotateDegrees(projectileDirection, angleOffsetDegrees));
            ShotProjectile projectile;
            projectile.damage = Damage{ definition_.damage, DamageType::Physical, definition_.impactForce };
            projectile.velocity = {
                pelletDirection.x * definition_.bulletSpeed,
                pelletDirection.y * definition_.bulletSpeed
            };
            projectile.lifetimeSeconds = projectileLifetimeSeconds;
            projectile.hitRadius = projectileHitRadius;
            result.projectiles.push_back(projectile);
        }

        if (!result.projectiles.empty())
        {
            result.damage = result.projectiles.front().damage;
        }

        // 총기의 손맛/연출 수치는 발사 결과에 실어 씬 쪽 피드백 시스템으로 전달합니다.
        result.feedback = definition_.feedback;

        // 이 시점에도 Gun의 내부 상태는 바뀌지 않았습니다.
        // 탄약 감소와 쿨타임 설정은 TryFire에서만 처리합니다.
        return result;
    }

    bool Gun::CycleFireMode()
    {
        // 현재 SemiAuto라면 FullAuto로, 현재 FullAuto라면 SemiAuto로 전환을 시도합니다.
        // 해당 모드를 지원하지 않는 무기라면 현재 모드를 유지합니다.
        const FireControlMode nextMode = fireMode_ == FireControlMode::SemiAuto
            ? FireControlMode::FullAuto
            : FireControlMode::SemiAuto;

        if (!SupportsFireMode(nextMode))
        {
            return false;
        }

        fireMode_ = nextMode;
        return true;
    }

    // TryReload:
    // - 지상에서만 재장전을 시작합니다.
    // - 같은 재장전이라도 R키로 요청했는지, 빈 탄창 좌클릭으로 요청했는지에 따라 조건이 다릅니다.
    bool Gun::TryReload(bool grounded, ReloadIntent intent)
    {
        // 공중에서는 기본 재장전을 시작할 수 없습니다.
        // 나중에 "공중 재장전" 스킬이 생기면 이 조건을 스킬 쪽에서 우회하거나 별도 함수로 처리할 수 있습니다.
        if (!grounded)
        {
            return false;
        }

        // 이미 재장전 중이면 다시 시작하지 않습니다.
        // 재장전 시간을 덮어쓰면 입력 연타로 재장전 타이밍이 이상해질 수 있습니다.
        if (reloadRemaining_ > 0.0f)
        {
            return false;
        }

        // 탄창이 이미 가득 차 있으면 R키든 좌클릭이든 재장전할 이유가 없습니다.
        if (ammoInMagazine_ == definition_.magazineSize)
        {
            return false;
        }

        // 좌클릭 자동 재장전은 탄창이 완전히 비었을 때만 허용합니다.
        // 탄이 1발이라도 남아 있다면 좌클릭은 발사 입력으로 남겨야 합니다.
        if (intent == ReloadIntent::EmptyMagazineOnly && ammoInMagazine_ > 0)
        {
            return false;
        }

        reloadRemaining_ = definition_.reloadSeconds;
        return true;
    }

    void Gun::StartReload(float reloadSeconds)
    {
        if (reloadSeconds <= 0.0f)
        {
            FillMagazine();
            return;
        }

        reloadRemaining_ = reloadSeconds;
    }

    bool Gun::NeedsReload() const
    {
        return ammoInMagazine_ < definition_.magazineSize;
    }

    void Gun::FillMagazine()
    {
        ammoInMagazine_ = definition_.magazineSize;
        reloadRemaining_ = 0.0f;
    }

    // Definition:
    // - 총의 기본 데이터를 반환합니다.
    const WeaponDefinition& Gun::Definition() const
    {
        return definition_;
    }

    FireControlMode Gun::FireMode() const
    {
        return fireMode_;
    }

    bool Gun::SupportsFireMode(FireControlMode mode) const
    {
        if (mode == FireControlMode::SemiAuto)
        {
            return definition_.supportsSemiAuto;
        }

        return definition_.supportsFullAuto;
    }

    // AmmoInMagazine:
    // - 현재 탄 수를 반환합니다.
    int Gun::AmmoInMagazine() const
    {
        return ammoInMagazine_;
    }

    // IsReloading:
    // - reloadRemaining_이 0보다 크면 아직 재장전 중입니다.
    bool Gun::IsReloading() const
    {
        return reloadRemaining_ > 0.0f;
    }

    FireControlMode Gun::ResolveSupportedFireMode(FireControlMode requestedMode) const
    {
        if (SupportsFireMode(requestedMode))
        {
            return requestedMode;
        }

        if (definition_.supportsSemiAuto)
        {
            return FireControlMode::SemiAuto;
        }

        if (definition_.supportsFullAuto)
        {
            return FireControlMode::FullAuto;
        }

        // 데이터가 완전히 잘못되어 둘 다 false인 경우에도 게임이 멈추지 않도록 SemiAuto를 기본값으로 둡니다.
        return FireControlMode::SemiAuto;
    }
}

