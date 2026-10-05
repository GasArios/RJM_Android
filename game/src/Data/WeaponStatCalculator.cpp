// WeaponStatCalculator.cpp
// - 무기 데이터에서 UI/도감용 파생 스탯을 계산합니다.
// - 실제 발사, 반동, 재장전 같은 게임 상태는 건드리지 않습니다.

#include "RecoilJumpMan/Data/WeaponStatCalculator.h"

#include <algorithm>
#include <cmath>

namespace
{
    float Clamp01(float value)
    {
        return std::clamp(value, 0.0f, 1.0f);
    }

    float Smooth01(float value)
    {
        const float t = Clamp01(value);
        return t * t * (3.0f - 2.0f * t);
    }

    float SafePositive(float value, float fallback)
    {
        return value > 0.0f ? value : fallback;
    }

    float RoundToOneDecimal(float value)
    {
        return std::round(value * 10.0f) / 10.0f;
    }
}

namespace rjm
{
    WeaponMobilityScoreBreakdown CalculateWeaponMobilityScoreBreakdown(const WeaponDefinition& weapon)
    {
        WeaponMobilityScoreBreakdown result;

        // 안전 보정:
        // - 데이터 파일에서 0초 쿨타임, 0초 재장전, 0발 탄창 같은 값이 들어와도 계산이 터지지 않게 합니다.
        // - 실제 게임 밸런스 검증은 별도 데이터 검증 단계에서 잡는 것이 좋고, 이 함수는 UI가 깨지지 않는 쪽에 집중합니다.
        const float recoil = std::max(0.0f, weapon.recoilForce);
        const float magazine = static_cast<float>(std::max(1, weapon.magazineSize));
        const float cooldown = SafePositive(weapon.fireCooldownSeconds, 0.05f);
        const float reload = SafePositive(weapon.reloadSeconds, 0.05f);
        const float altitudeResistance = Clamp01(weapon.altitudeDampingResistance);

        const float shotsPerSecond = 1.0f / cooldown;
        const float cycleSeconds = std::max(0.05f, magazine * cooldown + reload);
        const float reloadTax = reload / cycleSeconds;
        const float sustainedRecoilPerSecond = recoil * magazine / cycleSeconds;

        // recoilNorm:
        // - 350 이하는 이동용 반동으로 낮게 보고, 1500 부근은 현재 프로토타입의 강한 반동 상한으로 봅니다.
        // - Smooth01을 써서 중간 구간 변화가 부드럽고, 극단값은 점수 폭주가 덜 나게 합니다.
        result.recoilNorm = Smooth01((recoil - 350.0f) / (1500.0f - 350.0f));

        // magazineNorm:
        // - 장탄수는 많을수록 좋지만, 6발에서 10발로 늘어나는 체감과 26발에서 30발로 늘어나는 체감은 다릅니다.
        // - exp 곡선을 쓰면 초반 장탄수 증가의 의미는 크게, 과도한 장탄수의 점수 폭주는 작게 만들 수 있습니다.
        result.magazineNorm = Clamp01(1.0f - std::exp(-magazine / 8.0f));

        // cadenceNorm:
        // - 초당 발사 횟수를 기동성의 미세 조정 능력으로 봅니다.
        // - 너무 빠른 연사는 조작 피로/탄 낭비가 있으므로 8발/초 부근에서 충분히 높은 점수로 포화시킵니다.
        result.cadenceNorm = Smooth01((shotsPerSecond - 0.8f) / (8.0f - 0.8f));

        // reloadNorm:
        // - 전체 발사 사이클 중 재장전에 먹히는 비율이 낮을수록 좋은 점수입니다.
        // - 재장전 자체가 이 게임의 리듬이므로 0에 가까울수록 무조건 만점이 되지는 않게 구간을 둡니다.
        result.reloadNorm = 1.0f - Smooth01((reloadTax - 0.18f) / (0.65f - 0.18f));

        // altitudeNorm:
        // - 고도 감쇠 저항은 이미 0~1 설계값으로 취급합니다.
        result.altitudeNorm = altitudeResistance;

        // sustainedRecoilNorm:
        // - 장기적으로 1초마다 어느 정도의 반동 행동량을 만들 수 있는지입니다.
        // - 핸드캐논 같은 무기는 순간 반동은 높지만 이 항목에서는 재장전 부담 때문에 점수가 눌립니다.
        result.sustainedRecoilNorm = Smooth01((sustainedRecoilPerSecond - 900.0f) / (7000.0f - 900.0f));

        // burstMobility:
        // - 한 발로 길을 뚫는 능력입니다.
        // - 고도 감쇠 저항이 높으면 같은 반동력이라도 높은 절벽에서 더 가치가 있으므로 보너스를 줍니다.
        result.burstMobility = result.recoilNorm * (0.70f + 0.30f * result.altitudeNorm);

        // airControl:
        // - 공중에서 몸을 잔잔하게 고치는 능력입니다.
        // - 장탄수/연사력/재장전이 중요하고, 반동이 너무 크면 정밀 제어가 어려우므로 약한 페널티를 둡니다.
        result.airControl = (
            0.50f * result.magazineNorm
            + 0.35f * result.cadenceNorm
            + 0.15f * result.reloadNorm)
            * (1.0f - 0.10f * result.recoilNorm);

        // airEndurance:
        // - 한 번의 체공 중 얼마나 오래 의미 있는 선택지를 유지하는가입니다.
        // - 탄이 많고, 재장전 부담이 낮고, 높은 고도에서도 반동이 덜 죽는 총이 높게 나옵니다.
        result.airEndurance =
            0.35f * result.magazineNorm
            + 0.25f * result.reloadNorm
            + 0.40f * result.altitudeNorm;

        // sustainedMobility:
        // - 반복 플레이에서 계속 움직임을 생산하는 능력입니다.
        // - 같은 sustained recoil이라도 재장전 부담이 낮은 총을 더 좋게 봅니다.
        result.sustainedMobility = result.sustainedRecoilNorm * (0.60f + 0.40f * result.reloadNorm);

        result.rawScore = Clamp01(
            0.30f * result.burstMobility
            + 0.27f * result.airControl
            + 0.25f * result.airEndurance
            + 0.18f * result.sustainedMobility);

        result.finalScore = RoundToOneDecimal(result.rawScore * 10.0f);
        return result;
    }

    float CalculateWeaponMobilityScore(const WeaponDefinition& weapon)
    {
        return CalculateWeaponMobilityScoreBreakdown(weapon).finalScore;
    }
}

