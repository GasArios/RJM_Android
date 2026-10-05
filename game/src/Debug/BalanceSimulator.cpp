// BalanceSimulator.cpp
// - BalanceSimulator 클래스의 구현부입니다.
// - 현재는 무기 하나의 폭딜, 지속 DPS, 총 반동량을 계산합니다.

#include "RecoilJumpMan/Debug/BalanceSimulator.h"

#include <algorithm>

namespace rjm
{
    // EvaluateWeapon:
    // - weapon의 기본 수치를 이용해 간단한 성능 보고서를 만듭니다.
    WeaponSimulationReport BalanceSimulator::EvaluateWeapon(const WeaponDefinition& weapon) const
    {
        // magazine은 탄창 크기입니다.
        // std::max로 최소 1 이상이 되게 해서 0으로 나누는 실수를 막습니다.
        const float magazine = static_cast<float>(std::max(1, weapon.magazineSize));

        // 탄창을 모두 비우는 데 걸리는 시간입니다.
        const float fireTime = std::max(0.01f, weapon.fireCooldownSeconds) * magazine;

        // 한 사이클은 탄창을 비우는 시간 + 재장전 시간입니다.
        const float cycleTime = fireTime + std::max(0.0f, weapon.reloadSeconds);

        WeaponSimulationReport report;

        // 탄창을 다 맞췄을 때의 피해량입니다.
        report.burstDamage = weapon.damage * magazine;

        // 지속 DPS입니다.
        report.sustainedDps = report.burstDamage / std::max(0.01f, cycleTime);

        // 총 반동량입니다.
        report.totalRecoilImpulse = weapon.recoilForce * magazine;

        // 현재는 공중 재장전이 없다고 가정하므로 탄창 크기와 같습니다.
        report.airShotsBeforeReload = magazine;

        return report;
    }
}

