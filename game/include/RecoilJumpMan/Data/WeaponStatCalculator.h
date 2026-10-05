#pragma once

// WeaponStatCalculator.h
// - WeaponDefinition의 원본 수치를 플레이어에게 보여줄 보조 스탯으로 변환합니다.
// - 여기서 계산하는 기동력 점수는 UI/도감/밸런스 비교용이며, 실제 물리 엔진에는 직접 적용하지 않습니다.

#include "RecoilJumpMan/Data/WeaponDefinition.h"

namespace rjm
{
    // WeaponMobilityScoreBreakdown:
    // - 최종 기동력 점수가 어떤 항목에서 나왔는지 추적하기 위한 분해 결과입니다.
    // - UI는 finalScore만 보여줘도 되지만, 디버그 오버레이/밸런스 툴은 세부 항목을 보면 튜닝이 쉬워집니다.
    struct WeaponMobilityScoreBreakdown
    {
        // 0~1 정규화된 원천 항목입니다.
        float recoilNorm = 0.0f;
        float magazineNorm = 0.0f;
        float cadenceNorm = 0.0f;
        float reloadNorm = 0.0f;
        float altitudeNorm = 0.0f;
        float sustainedRecoilNorm = 0.0f;

        // 0~1 정규화된 중간 항목입니다.
        float burstMobility = 0.0f;
        float airControl = 0.0f;
        float airEndurance = 0.0f;
        float sustainedMobility = 0.0f;

        // 0~1 원본 종합 점수입니다.
        float rawScore = 0.0f;

        // 0~10 표시용 최종 점수입니다.
        // 소수점 한 자리까지 반올림된 값입니다.
        float finalScore = 0.0f;
    };

    // CalculateWeaponMobilityScoreBreakdown:
    // - 장탄수, 연사력, 재장전, 반동력, 고도 감쇠 저항을 종합해 기동력 점수를 계산합니다.
    // - 점수는 "이 총으로 공중에서 얼마나 편하고 유의미하게 움직일 수 있는가"를 나타냅니다.
    WeaponMobilityScoreBreakdown CalculateWeaponMobilityScoreBreakdown(const WeaponDefinition& weapon);

    // CalculateWeaponMobilityScore:
    // - 최종 0~10 기동력 점수만 필요할 때 쓰는 편의 함수입니다.
    float CalculateWeaponMobilityScore(const WeaponDefinition& weapon);
}

