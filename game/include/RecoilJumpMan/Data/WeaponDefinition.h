#pragma once

// WeaponDefinition.h
// - 총기 하나의 기본 능력치를 담는 데이터 구조입니다.
// - "총기의 동작 코드"가 아니라, 총의 수치와 분류를 담는 설계도입니다.
// - 나중에는 assets/data/weapons/*.json 같은 파일에서 이 구조로 읽어올 수 있습니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <string>

namespace rjm
{
    // WeaponFeedbackProfile:
    // - 데미지/반동력 같은 밸런스 수치와 별도로, 손맛과 연출을 조절하는 값입니다.
    // - 발사 순간과 명중 순간을 분리해두면 나중에 보스 약점, 폭발, 패링에도 같은 구조를 쓸 수 있습니다.
    struct WeaponFeedbackProfile
    {
        // 발사 성공 순간 게임플레이 시간을 멈추는 시간입니다.
        // Sleep처럼 프로그램을 멈추는 시간이 아니라, 플레이어/적/총알 업데이트에 들어가는 delta만 줄입니다.
        // 0.02~0.04 정도는 가벼운 총, 0.08 이상은 아주 묵직한 총에 어울립니다.
        float fireHitStopSeconds = 0.0f;

        // 발사 성공 순간 화면 흔들림 강도입니다. 화면 픽셀 단위의 최대 흔들림으로 취급합니다.
        // 이 값은 데미지나 반동력에 자동 비례하지 않고, 손맛 튜닝용으로 독립되어 있습니다.
        float fireShakeStrength = 0.0f;

        // 발사 성공 순간 화면 흔들림 지속 시간입니다.
        // 너무 길면 조준 피로가 커지므로 연사 무기는 특히 짧게 잡는 편이 좋습니다.
        float fireShakeSeconds = 0.0f;

        // 발사 흔들림의 진동 빈도입니다.
        // 값이 높으면 빠르게 떨리고, 낮으면 묵직하게 흔들립니다.
        float fireShakeFrequency = 30.0f;

        // 탄환이 적/오브젝트에 명중했을 때 쓸 역경직 시간입니다.
        // 발사 피드백과 분리되어 있으므로, 쐈을 때 약하고 맞췄을 때 강한 무기도 만들 수 있습니다.
        float hitStopSeconds = 0.0f;

        // 탄환이 적/오브젝트에 명중했을 때 쓸 흔들림 강도입니다.
        // 나중에 보스나 장갑 오브젝트는 이 값을 보정해서 과한 흔들림을 줄일 수 있습니다.
        float hitShakeStrength = 0.0f;

        // 탄환이 적/오브젝트에 명중했을 때 쓸 흔들림 지속 시간입니다.
        // 폭발탄이나 핸드캐논처럼 명중감이 중요한 무기는 발사 흔들림보다 길게 줄 수 있습니다.
        float hitShakeSeconds = 0.0f;

        // 명중 흔들림의 진동 빈도입니다.
        // 발사와 명중의 주파수를 다르게 두면 "쏘는 맛"과 "맞는 맛"을 구분할 수 있습니다.
        float hitShakeFrequency = 34.0f;
    };

    // enum class:
    // - 관련 있는 상수들을 묶는 C++ 문법입니다.
    // - enum class는 일반 enum보다 이름 충돌이 적고 타입이 더 안전합니다.
    enum class WeaponType
    {
        Unknown,         // 아직 분류되지 않은 무기입니다.
        Revolver,        // 리볼버 계열입니다.
        Smg,             // 기관단총 계열입니다.
        Shotgun,         // 산탄총 계열입니다.
        BoltAction,      // 볼트액션 계열입니다.
        LeverAction,     // 레버액션 계열입니다.
        HandCannon,      // 핸드캐논 계열입니다.
        GatlingGun,      // 개틀링 건 계열입니다.
        AssaultRifle,    // AR 계열입니다.
        RocketLauncher,  // 로켓런처 계열입니다.
        HarpoonGun,      // 작살총/와이어 이동 계열입니다.
        GrenadeLauncher, // 유탄 발사기 계열입니다.
        Mortar,          // 박격포/곡사포 계열입니다.
        Melee,           // 검, 총검, 근접 무기 계열입니다.
        ChargeRifle,     // 차지 라이플 계열입니다.
        Flamethrower,    // 화염방사기 계열입니다.
        MagicScroll,     // 마법 스크롤/마도구 계열입니다.
        Explosive        // 폭발탄, 특수 폭발 계열입니다.
    };

    // WeaponAttackPattern:
    // - WeaponType이 기획/표시용 분류라면, 이 값은 실제 발사 로직의 큰 형태입니다.
    // - 같은 Shotgun 타입이라도 PelletSpread/SingleProjectile로 다르게 만들 수 있고,
    //   MagicScroll 타입도 SingleProjectile, AreaBurst 등으로 확장할 수 있습니다.
    enum class WeaponAttackPattern
    {
        SingleProjectile,     // 한 발 직선 투사체입니다.
        AutoProjectile,       // 연사형 직선 투사체입니다. 실제 반복은 FireControlMode가 담당합니다.
        PelletSpread,         // 한 번 발사에 여러 짧은 사거리 펠릿을 퍼뜨립니다.
        ExplosiveProjectile,  // 로켓처럼 명중/타일 충돌 시 폭발할 투사체입니다.
        ArcBounceProjectile,  // 중력/바운스가 붙을 유탄류 예약값입니다.
        TetherProjectile,     // 작살총처럼 맞은 지점으로 이동하는 투사체 예약값입니다.
        ConeHit,              // 부채꼴 순간 타격 예약값입니다.
        MeleeArc,             // 검/총검 휘두르기 예약값입니다.
        ContinuousCone,       // 화염방사기처럼 유지되는 부채꼴 공격 예약값입니다.
        ChargeProjectile,     // 차지 시간에 따라 성능이 달라지는 투사체 예약값입니다.
        MagicCast             // 스크롤/마법 발동 예약값입니다.
    };

    // FireControlMode:
    // - 플레이어가 좌클릭 입력을 어떻게 발사로 해석할지 나타냅니다.
    // - SemiAuto는 클릭한 순간만 발사하고, FullAuto는 누르고 있는 동안 쿨타임마다 발사합니다.
    enum class FireControlMode
    {
        SemiAuto,
        FullAuto
    };

    // struct:
    // - 단순 데이터 묶음에 적합합니다.
    // - class와 비슷하지만 멤버가 기본 public입니다.
    struct WeaponDefinition
    {
        // 데이터 파일이나 코드에서 무기를 식별하는 고유 id입니다.
        DefinitionId id;

        // 화면에 표시할 무기 이름입니다.
        std::string displayName;

        // 무기 분류입니다.
        WeaponType type = WeaponType::Unknown;

        // 실제 발사 패턴입니다.
        WeaponAttackPattern attackPattern = WeaponAttackPattern::SingleProjectile;

        // 적에게 주는 기본 피해량입니다.
        // PelletSpread에서는 펠릿 1개당 피해량으로 해석합니다.
        float damage = 0.0f;

        // 발사 시 플레이어가 받는 반동 힘입니다.
        // 이 게임에서 이동 능력의 핵심 수치입니다.
        float recoilForce = 0.0f;

        // 고도 감쇠 저항입니다.
        // 0이면 공중 상승 높이에 따른 수직 반동 감쇠를 기본 곡선 그대로 받고,
        // 1에 가까울수록 높은 고도에서도 위쪽 반동 효율이 덜 깎입니다.
        // 실제 물리에 쓰이는 이동 해금/이동 특화 스탯입니다.
        float altitudeDampingResistance = 0.0f;

        // 벽, 방패, 오브젝트 등에 전달되는 충격 힘입니다.
        float impactForce = 0.0f;

        // 한 탄창에 들어가는 탄 수입니다.
        int magazineSize = 1;

        // 지상 재장전에 걸리는 시간입니다.
        float reloadSeconds = 1.0f;

        // 한 발 쏜 뒤 다음 발을 쏘기까지 기다려야 하는 시간입니다.
        float fireCooldownSeconds = 0.2f;

        // 기본 발사 모드입니다.
        // 저장 데이터가 생기기 전에는 총을 장착할 때 이 모드로 시작합니다.
        FireControlMode defaultFireMode = FireControlMode::SemiAuto;

        // SemiAuto 모드를 지원하는지 나타냅니다.
        // 샷건, 볼트액션, 핸드캐논처럼 한 발씩 조심해서 쏘는 무기는 보통 true입니다.
        bool supportsSemiAuto = true;

        // FullAuto 모드를 지원하는지 나타냅니다.
        // SMG나 편의성을 준 리볼버처럼 누르고 있으면 반복 발사 가능한 무기에 사용합니다.
        bool supportsFullAuto = false;

        // 탄환이 날아가는 속도입니다.
        float bulletSpeed = 900.0f;

        // 탄이 퍼지는 각도입니다.
        float spreadDegrees = 0.0f;

        // 한 번의 발사에서 생성할 투사체 수입니다.
        // SingleProjectile 계열은 보통 1, PelletSpread 계열은 5~9 정도를 사용합니다.
        int projectileCount = 1;

        // 투사체가 살아있는 시간입니다.
        // projectileRange가 0 이하일 때 이 값을 그대로 씁니다.
        float projectileLifetimeSeconds = 2.0f;

        // 투사체 최대 사거리입니다.
        // 0 이하이면 projectileLifetimeSeconds를 사용하고,
        // 양수이면 bulletSpeed와 조합해 lifetime을 자동 계산합니다.
        float projectileRange = 0.0f;

        // 투사체 충돌 반지름입니다.
        float projectileHitRadius = 4.0f;

        // aimAssistEnabled:
        // - 이 무기가 조준 보정을 사용할 수 있는지 나타냅니다.
        // - 핸드캐논처럼 "정확히 직접 맞춰야 하는" 고난도 무기는 false로 둘 수 있습니다.
        bool aimAssistEnabled = true;

        // aimAssistRadiusPixels:
        // - 마우스 포인터 기준 보정 반경입니다.
        // - 화면 픽셀 기준이라 카메라 줌이 바뀌어도 플레이어가 느끼는 보정 범위는 비슷하게 유지됩니다.
        float aimAssistRadiusPixels = 42.0f;

        // aimAssistMaxCorrectionDegrees:
        // - 원래 마우스 조준 방향에서 적 방향으로 얼마나 꺾을 수 있는지 제한합니다.
        // - 값이 너무 크면 이동하려고 쏜 탄이 적에게 빨려 들어가는 느낌이 생기므로 무기별로 조심해서 조절합니다.
        float aimAssistMaxCorrectionDegrees = 15.0f;

        // aimAssistUsesAssistedRecoil:
        // - true이면 탄 방향과 반동 방향을 둘 다 보정된 목표 기준으로 계산합니다.
        // - false이면 탄은 적에게 보정하지만, 반동은 원래 마우스 방향을 유지합니다.
        // - 이동과 공격을 더 강하게 분리하고 싶은 무기/옵션에서 사용할 수 있는 확장 지점입니다.
        bool aimAssistUsesAssistedRecoil = true;

        // 최대 내구도입니다.
        int maxDurability = 100;

        // 장비 무게입니다. 기획서의 총 무게 시스템에 사용됩니다.
        int weight = 1;

        // 장착 가능한 인챈트 슬롯 수입니다.
        int enchantmentSlots = 0;

        // 총기의 손맛/연출 수치입니다.
        // 데미지, 반동력, 충격력과 분리해서 밸런스 수정이 조작감까지 흔들지 않게 합니다.
        WeaponFeedbackProfile feedback;
    };
}

