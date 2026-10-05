#pragma once

// Health.h
// - 체력을 관리하는 클래스 선언부입니다.
// - 현재 체력, 최대 체력, 피해, 회복, 사망 여부를 담당합니다.

namespace rjm
{
    class Health
    {
    public:
        // explicit:
        // - float 값이 자동으로 Health 객체로 변환되는 것을 막습니다.
        // - 생성자는 최대 체력을 받아 current와 maximum을 설정합니다.
        explicit Health(float maximum = 1.0f);

        // Damage:
        // - 체력을 amount만큼 감소시킵니다.
        void Damage(float amount);

        // Heal:
        // - 체력을 amount만큼 회복합니다.
        void Heal(float amount);

        // SetMaximum:
        // - 최대 체력을 변경합니다.
        // - fill이 true면 현재 체력도 최대치까지 채웁니다.
        void SetMaximum(float maximum, bool fill);

        // Current:
        // - 현재 체력을 반환합니다.
        float Current() const;

        // Maximum:
        // - 최대 체력을 반환합니다.
        float Maximum() const;

        // IsDead:
        // - 현재 체력이 0 이하인지 반환합니다.
        bool IsDead() const;

    private:
        // 현재 체력입니다.
        float current_ = 1.0f;

        // 최대 체력입니다.
        float maximum_ = 1.0f;
    };
}

