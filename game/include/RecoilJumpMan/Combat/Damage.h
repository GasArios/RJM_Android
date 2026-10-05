#pragma once

// Damage.h
// - 공격이 전달하는 피해 정보를 담는 구조입니다.
// - 단순히 숫자 하나가 아니라 피해량, 피해 종류, 충격력을 함께 표현합니다.

namespace rjm
{
    // DamageType:
    // - 피해 속성을 구분합니다.
    enum class DamageType
    {
        Physical,   // 물리 피해입니다.
        Fire,       // 화염 피해입니다.
        Explosive,  // 폭발 피해입니다.
        TrueDamage  // 방어력 등을 무시하는 고정 피해입니다.
    };

    struct Damage
    {
        // 실제 체력을 깎는 수치입니다.
        float amount = 0.0f;

        // 피해의 종류입니다.
        DamageType type = DamageType::Physical;

        // 적이나 오브젝트를 밀어내는 힘입니다.
        float impactForce = 0.0f;
    };

    struct DamageResult
    {
        // 실제로 피해 처리가 받아들여졌는지 나타냅니다.
        // 무적, 방패, 이미 죽은 대상처럼 피해가 무시된 경우 false가 될 수 있습니다.
        bool accepted = false;

        // 요청된 피해량입니다.
        float requestedAmount = 0.0f;

        // 방어력, 내성, 약점 보정 후 실제로 적용된 피해량입니다.
        float appliedAmount = 0.0f;

        // 피해 처리 후 남은 체력입니다.
        float remainingHealth = 0.0f;

        // 이 피해로 대상이 사망했는지 나타냅니다.
        bool killed = false;
    };
}

