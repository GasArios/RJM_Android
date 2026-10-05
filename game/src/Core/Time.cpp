// Time.cpp
// - Time 클래스의 실제 구현부입니다.

#include "RecoilJumpMan/Core/Time.h"

namespace rjm
{
    // Update:
    // - deltaSeconds를 저장하고, 전체 시간 totalSeconds_에 더합니다.
    // - +=는 기존 값에 오른쪽 값을 더해서 다시 저장하는 연산자입니다.
    void Time::Update(float deltaSeconds)
    {
        deltaSeconds_ = deltaSeconds;
        totalSeconds_ += deltaSeconds;
    }

    // DeltaSeconds:
    // - const가 붙은 멤버 함수는 객체의 멤버 변수를 수정하지 않겠다는 약속입니다.
    float Time::DeltaSeconds() const
    {
        return deltaSeconds_;
    }

    // TotalSeconds:
    // - 게임이 실행된 총 시간을 반환합니다.
    float Time::TotalSeconds() const
    {
        return totalSeconds_;
    }
}

