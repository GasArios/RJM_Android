#include "RecoilJumpMan/Data/BuiltInData.h"

namespace rjm
{
    const std::vector<WeaponDefinition>& BuiltInWeaponDefinitions()
    {
        static const std::vector<WeaponDefinition> weapons = []
        {
            std::vector<WeaponDefinition> result;

            // Old Revolver:
            // - 기준점 역할의 기본 무기입니다.
            // - 적당한 반동, 적당한 탄속, 적당한 탄창을 가져 다른 무기의 장단점을 비교하는 기준으로 씁니다.
            WeaponDefinition revolver;
            revolver.id = "old_revolver";
            revolver.displayName = "Old Revolver";
            revolver.type = WeaponType::Revolver;
            revolver.attackPattern = WeaponAttackPattern::SingleProjectile;
            revolver.damage = 12.0f;
            revolver.recoilForce = 600.0f;
            revolver.altitudeDampingResistance = 0.05f;
            revolver.impactForce = 8.0f;
            revolver.magazineSize = 6;
            revolver.reloadSeconds = 0.4f;
            revolver.fireCooldownSeconds = 0.3f;
            revolver.defaultFireMode = FireControlMode::SemiAuto;
            revolver.supportsSemiAuto = true;
            revolver.supportsFullAuto = true;
            revolver.bulletSpeed = 900.0f;
            revolver.projectileRange = 900.0f;
            revolver.weight = 1;
            revolver.feedback.fireHitStopSeconds = 0.065f;
            revolver.feedback.fireShakeStrength = 11.0f;
            revolver.feedback.fireShakeSeconds = 0.08f;
            revolver.feedback.fireShakeFrequency = 34.0f;
            revolver.feedback.hitStopSeconds = 0.1f;
            revolver.feedback.hitShakeStrength = 4.0f;
            revolver.feedback.hitShakeSeconds = 0.09f;
            result.push_back(revolver);

            // Recoil SMG:
            // - 낮은 데미지와 낮은 반동을 빠르게 반복하는 공중 미세 조정용 무기입니다.
            // - 단발 화력보다 체공 제어와 움직이는 적에게 안정적으로 압박하는 역할을 우선합니다.
            WeaponDefinition smg;
            smg.id = "recoil_smg";
            smg.displayName = "Recoil SMG";
            smg.type = WeaponType::Smg;
            smg.attackPattern = WeaponAttackPattern::AutoProjectile;
            smg.damage = 5.0f;
            smg.recoilForce = 180.0f;
            smg.altitudeDampingResistance = 0.15f;
            smg.impactForce = 3.0f;
            smg.magazineSize = 24;
            smg.reloadSeconds = 0.55f;
            smg.fireCooldownSeconds = 0.07f;
            smg.defaultFireMode = FireControlMode::FullAuto;
            smg.supportsSemiAuto = true;
            smg.supportsFullAuto = true;
            smg.bulletSpeed = 760.0f;
            smg.spreadDegrees = 5.0f;
            smg.projectileRange = 620.0f;
            smg.projectileHitRadius = 3.0f;
            smg.weight = 2;
            smg.feedback.fireHitStopSeconds = 0.02f;
            smg.feedback.fireShakeStrength = 3.5f;
            smg.feedback.fireShakeSeconds = 0.04f;
            smg.feedback.hitStopSeconds = 0.025f;
            smg.feedback.hitShakeStrength = 1.5f;
            smg.feedback.hitShakeSeconds = 0.035f;
            result.push_back(smg);

            // Hunter Shotgun:
            // - 진짜 샷건 테스트용 데이터입니다.
            // - 한 번 발사에 펠릿 여러 개를 만들고, 각 펠릿은 짧은 수명으로 사거리가 제한됩니다.
            // - 반동은 발사 1회 기준으로만 적용되므로 펠릿 수가 이동력을 여러 번 증폭하지 않습니다.
            WeaponDefinition shotgun;
            shotgun.id = "hunter_shotgun";
            shotgun.displayName = "Hunter Shotgun";
            shotgun.type = WeaponType::Shotgun;
            shotgun.attackPattern = WeaponAttackPattern::PelletSpread;
            shotgun.damage = 6.0f;
            shotgun.recoilForce = 950.0f;
            shotgun.altitudeDampingResistance = 0.45f;
            shotgun.impactForce = 4.0f;
            shotgun.magazineSize = 2;
            shotgun.reloadSeconds = 0.75f;
            shotgun.fireCooldownSeconds = 0.35f;
            shotgun.defaultFireMode = FireControlMode::SemiAuto;
            shotgun.supportsSemiAuto = true;
            shotgun.supportsFullAuto = false;
            shotgun.bulletSpeed = 820.0f;
            shotgun.spreadDegrees = 28.0f;
            shotgun.projectileCount = 7;
            shotgun.projectileRange = 285.0f;
            shotgun.projectileHitRadius = 3.5f;
            shotgun.weight = 2;
            shotgun.feedback.fireHitStopSeconds = 0.12f;
            shotgun.feedback.fireShakeStrength = 18.0f;
            shotgun.feedback.fireShakeSeconds = 0.24f;
            shotgun.feedback.fireShakeFrequency = 21.0f;
            shotgun.feedback.hitStopSeconds = 0.25f;
            shotgun.feedback.hitShakeStrength = 9.0f;
            shotgun.feedback.hitShakeSeconds = 0.16f;
            result.push_back(shotgun);

            // ST-479 Bolt Action:
            // - 기존 "긴 사거리 단발 고화력" 샷건 테스트 감각을 넘겨받는 무기입니다.
            // - 펠릿이 아니라 한 발의 정확도와 탄속, 큰 반동으로 승부합니다.
            WeaponDefinition boltAction;
            boltAction.id = "st_479_bolt_action";
            boltAction.displayName = "ST-479 Bolt Action";
            boltAction.type = WeaponType::BoltAction;
            boltAction.attackPattern = WeaponAttackPattern::SingleProjectile;
            boltAction.damage = 42.0f;
            boltAction.recoilForce = 1120.0f;
            boltAction.altitudeDampingResistance = 0.52f;
            boltAction.impactForce = 24.0f;
            boltAction.magazineSize = 1;
            boltAction.reloadSeconds = 0.95f;
            boltAction.fireCooldownSeconds = 0.65f;
            boltAction.defaultFireMode = FireControlMode::SemiAuto;
            boltAction.supportsSemiAuto = true;
            boltAction.supportsFullAuto = false;
            boltAction.bulletSpeed = 1350.0f;
            boltAction.projectileRange = 1300.0f;
            boltAction.projectileHitRadius = 4.0f;
            boltAction.weight = 3;
            boltAction.feedback.fireHitStopSeconds = 0.16f;
            boltAction.feedback.fireShakeStrength = 24.0f;
            boltAction.feedback.fireShakeSeconds = 0.22f;
            boltAction.feedback.hitStopSeconds = 0.22f;
            boltAction.feedback.hitShakeStrength = 13.0f;
            boltAction.feedback.hitShakeSeconds = 0.18f;
            result.push_back(boltAction);

            // Frontier Lever Action:
            // - 볼트액션보다 빠르고 리볼버보다 묵직한 중거리 반복 사격 무기입니다.
            // - "강한 한 발"과 "빠른 연사" 사이의 리듬형 선택지로 둡니다.
            WeaponDefinition leverAction;
            leverAction.id = "frontier_lever_action";
            leverAction.displayName = "Frontier Lever Action";
            leverAction.type = WeaponType::LeverAction;
            leverAction.attackPattern = WeaponAttackPattern::SingleProjectile;
            leverAction.damage = 20.0f;
            leverAction.recoilForce = 760.0f;
            leverAction.altitudeDampingResistance = 0.22f;
            leverAction.impactForce = 11.0f;
            leverAction.magazineSize = 5;
            leverAction.reloadSeconds = 0.55f;
            leverAction.fireCooldownSeconds = 0.22f;
            leverAction.defaultFireMode = FireControlMode::SemiAuto;
            leverAction.supportsSemiAuto = true;
            leverAction.supportsFullAuto = false;
            leverAction.bulletSpeed = 1050.0f;
            leverAction.projectileRange = 980.0f;
            leverAction.weight = 2;
            leverAction.feedback.fireHitStopSeconds = 0.08f;
            leverAction.feedback.fireShakeStrength = 12.0f;
            leverAction.feedback.fireShakeSeconds = 0.09f;
            leverAction.feedback.hitStopSeconds = 0.11f;
            leverAction.feedback.hitShakeStrength = 5.0f;
            leverAction.feedback.hitShakeSeconds = 0.09f;
            result.push_back(leverAction);

            // Debug Hand Cannon:
            // - 극단적인 반동과 큰 단발 피해를 확인하기 위한 후반/디버그 성격의 무기입니다.
            // - 핸드캐논 계열은 나중에 벽 파괴, 고난도 절벽 등반 같은 해금 능력과 연결하기 좋습니다.
            WeaponDefinition handCannon;
            handCannon.id = "debug_hand_cannon";
            handCannon.displayName = "Debug Hand Cannon";
            handCannon.type = WeaponType::HandCannon;
            handCannon.attackPattern = WeaponAttackPattern::SingleProjectile;
            handCannon.damage = 60.0f;
            handCannon.recoilForce = 1500.0f;
            handCannon.altitudeDampingResistance = 0.70f;
            handCannon.impactForce = 36.0f;
            handCannon.magazineSize = 1;
            handCannon.reloadSeconds = 1.15f;
            handCannon.fireCooldownSeconds = 0.75f;
            handCannon.defaultFireMode = FireControlMode::SemiAuto;
            handCannon.supportsSemiAuto = true;
            handCannon.supportsFullAuto = false;
            handCannon.bulletSpeed = 980.0f;
            handCannon.projectileRange = 1050.0f;
            handCannon.projectileHitRadius = 5.0f;
            handCannon.weight = 4;
            handCannon.feedback.fireHitStopSeconds = 0.31f;
            handCannon.feedback.fireShakeStrength = 43.0f;
            handCannon.feedback.fireShakeSeconds = 0.5f;
            handCannon.feedback.fireShakeFrequency = 11.0f;
            handCannon.feedback.hitStopSeconds = 0.5f;
            handCannon.feedback.hitShakeStrength = 34.0f;
            handCannon.feedback.hitShakeSeconds = 0.50f;
            result.push_back(handCannon);

            // Portable Gatling:
            // - 매우 낮은 단발 반동을 고속으로 누적하는 지속 화력 무기입니다.
            // - 무겁고 탄창 의존도가 높기 때문에 이동 해금보다는 보스전 제압/압박 쪽에 가깝습니다.
            WeaponDefinition gatlingGun;
            gatlingGun.id = "portable_gatling";
            gatlingGun.displayName = "Portable Gatling";
            gatlingGun.type = WeaponType::GatlingGun;
            gatlingGun.attackPattern = WeaponAttackPattern::AutoProjectile;
            gatlingGun.damage = 4.0f;
            gatlingGun.recoilForce = 95.0f;
            gatlingGun.altitudeDampingResistance = 0.05f;
            gatlingGun.impactForce = 3.0f;
            gatlingGun.magazineSize = 80;
            gatlingGun.reloadSeconds = 1.35f;
            gatlingGun.fireCooldownSeconds = 0.045f;
            gatlingGun.defaultFireMode = FireControlMode::FullAuto;
            gatlingGun.supportsSemiAuto = false;
            gatlingGun.supportsFullAuto = true;
            gatlingGun.bulletSpeed = 880.0f;
            gatlingGun.spreadDegrees = 7.0f;
            gatlingGun.projectileRange = 780.0f;
            gatlingGun.projectileHitRadius = 3.0f;
            gatlingGun.weight = 4;
            gatlingGun.feedback.fireHitStopSeconds = 0.01f;
            gatlingGun.feedback.fireShakeStrength = 4.5f;
            gatlingGun.feedback.fireShakeSeconds = 0.035f;
            gatlingGun.feedback.hitStopSeconds = 0.02f;
            gatlingGun.feedback.hitShakeStrength = 1.5f;
            gatlingGun.feedback.hitShakeSeconds = 0.03f;
            result.push_back(gatlingGun);

            return result;
        }();

        return weapons;
    }

    const std::vector<EnemyDefinition>& BuiltInEnemyDefinitions()
    {
        static const std::vector<EnemyDefinition> enemies = []
        {
            std::vector<EnemyDefinition> result;

            EnemyDefinition debugSlime;
            debugSlime.id = "debug_slime";
            debugSlime.displayName = "Debug Slime";
            debugSlime.rank = EnemyRank::Normal;
            debugSlime.maxHealth = 20.0f;
            debugSlime.contactDamage = 8.0f;
            debugSlime.moveSpeed = 80.0f;
            debugSlime.bodySize = 28.0f;
            debugSlime.crowdSeparationAllowedOverlapRatio = 0.30f;
            debugSlime.crowdSeparationStrength = 0.55f;
            debugSlime.crowdSeparationMaxPush = 5.0f;
            debugSlime.ai.approachStyle = EnemyApproachStyle::Walk;
            debugSlime.ai.attackStyle = EnemyAttackStyle::Contact;
            debugSlime.ai.pressureTarget = EnemyPressureTarget::PlayerCurrentPosition;
            debugSlime.ai.reactionSpeed = EnemyReactionSpeed::Slow;
            debugSlime.ai.telegraphProfile = EnemyTelegraphProfile::Readable;
            debugSlime.ai.recoveryProfile = EnemyRecoveryProfile::Normal;
            debugSlime.ai.antiAirLevel = EnemyAntiAirLevel::Ignore;
            debugSlime.ai.requiresLineOfSight = true;
            debugSlime.ai.canWander = true;
            debugSlime.ai.detectRange = 620.0f;
            debugSlime.ai.leashRange = 1000.0f;
            debugSlime.ai.preferredRange = 28.0f;
            debugSlime.ai.pursuitMemorySeconds = 0.9f;
            debugSlime.ai.wanderSpeedMultiplier = 0.32f;
            debugSlime.ai.wanderRadius = 120.0f;
            debugSlime.ai.wanderSeconds = 1.0f;
            debugSlime.ai.idleSeconds = 0.85f;
            result.push_back(debugSlime);

            EnemyDefinition debugHound;
            debugHound.id = "debug_hound";
            debugHound.displayName = "Debug Hound";
            debugHound.rank = EnemyRank::Normal;
            debugHound.maxHealth = 16.0f;
            debugHound.contactDamage = 10.0f;
            debugHound.moveSpeed = 115.0f;
            debugHound.bodySize = 26.0f;
            debugHound.crowdSeparationAllowedOverlapRatio = 0.18f;
            debugHound.crowdSeparationStrength = 0.75f;
            debugHound.crowdSeparationMaxPush = 7.0f;
            debugHound.aimAssistRadiusPixels = 46.0f;
            debugHound.ai.approachStyle = EnemyApproachStyle::Run;
            debugHound.ai.attackStyle = EnemyAttackStyle::JumpPounce;
            debugHound.ai.pressureTarget = EnemyPressureTarget::PredictedLandingPosition;
            debugHound.ai.reactionSpeed = EnemyReactionSpeed::Fast;
            debugHound.ai.telegraphProfile = EnemyTelegraphProfile::LongStrong;
            debugHound.ai.recoveryProfile = EnemyRecoveryProfile::LargeOpening;
            debugHound.ai.antiAirLevel = EnemyAntiAirLevel::Weak;
            debugHound.ai.requiresLineOfSight = true;
            debugHound.ai.canWander = true;
            debugHound.ai.detectRange = 760.0f;
            debugHound.ai.leashRange = 1300.0f;
            debugHound.ai.preferredRange = 90.0f;
            debugHound.ai.attackRange = 210.0f;
            debugHound.ai.pursuitMemorySeconds = 1.25f;
            debugHound.ai.wanderSpeedMultiplier = 0.45f;
            debugHound.ai.wanderRadius = 190.0f;
            debugHound.ai.wanderSeconds = 1.25f;
            debugHound.ai.idleSeconds = 0.65f;
            debugHound.ai.attackCooldownSeconds = 1.6f;
            debugHound.ai.attackActiveSeconds = 0.35f;
            debugHound.ai.landingPredictionSeconds = 0.42f;
            debugHound.ai.jumpPounceHorizontalSpeed = 430.0f;
            debugHound.ai.jumpPounceVerticalSpeed = 360.0f;
            result.push_back(debugHound);

            return result;
        }();

        return enemies;
    }
}

