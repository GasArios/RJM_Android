// Health.cpp
// - Health 클래스의 구현부입니다.
// - 체력 감소/회복 시 0 아래나 최대 체력 위로 벗어나지 않게 제한합니다.

#include "RecoilJumpMan/Combat/Health.h"

#include <algorithm>

namespace rjm
{
    // 생성자:
    // - current_와 maximum_을 둘 다 maximum 값으로 초기화합니다.
    Health::Health(float maximum)
        : current_(maximum), maximum_(maximum)
    {
    }

    // Damage:
    // - std::max를 사용해 체력이 0보다 작아지지 않게 합니다.
    void Health::Damage(float amount)
    {
        current_ = std::max(0.0f, current_ - amount);
    }

    // Heal:
    // - std::min을 사용해 체력이 최대 체력을 넘지 않게 합니다.
    void Health::Heal(float amount)
    {
        current_ = std::min(maximum_, current_ + amount);
    }

    // SetMaximum:
    // - 최대 체력을 최소 1.0 이상으로 제한합니다.
    void Health::SetMaximum(float maximum, bool fill)
    {
        maximum_ = std::max(1.0f, maximum);

        // fill이 true면 현재 체력을 최대 체력으로 채우고,
        // false면 현재 체력을 유지하되 새 최대 체력을 넘지 않게 합니다.
        current_ = fill ? maximum_ : std::min(current_, maximum_);
    }

    // Current:
    // - 현재 체력을 반환합니다.
    float Health::Current() const
    {
        return current_;
    }

    // Maximum:
    // - 최대 체력을 반환합니다.
    float Health::Maximum() const
    {
        return maximum_;
    }

    // IsDead:
    // - 체력이 0 이하이면 true입니다.
    bool Health::IsDead() const
    {
        return current_ <= 0.0f;
    }
}

