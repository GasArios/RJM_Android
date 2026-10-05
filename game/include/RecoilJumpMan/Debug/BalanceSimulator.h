#pragma once

// BalanceSimulator.h
// - 무기 밸런스를 계산하기 위한 개발용 도구의 시작점입니다.
// - 기획서에서 말한 자동 시뮬레이션 도구로 확장될 수 있습니다.

#include "RecoilJumpMan/Data/WeaponDefinition.h"

namespace rjm
{
    // WeaponSimulationReport:
    // - 무기 하나를 간단히 계산한 결과를 담는 구조체입니다.
    struct WeaponSimulationReport
    {
        // 탄창 하나를 전부 맞췄을 때의 총 피해량입니다.
        float burstDamage = 0.0f;

        // 재장전까지 포함한 평균 DPS입니다.
        float sustainedDps = 0.0f;

        // 탄창 전체를 쏠 때 발생하는 총 반동량입니다.
        float totalRecoilImpulse = 0.0f;

        // 재장전 없이 공중에서 쏠 수 있는 발사 횟수입니다.
        float airShotsBeforeReload = 0.0f;
    };

    class BalanceSimulator
    {
    public:
        // EvaluateWeapon:
        // - WeaponDefinition 하나를 받아 간단한 밸런스 수치를 계산합니다.
        WeaponSimulationReport EvaluateWeapon(const WeaponDefinition& weapon) const;
    };
}

