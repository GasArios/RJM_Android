#pragma once

// Time.h
// - 프레임 시간과 누적 시간을 관리하는 Time 클래스의 선언부입니다.
// - 게임에서는 "초당 몇 픽셀 이동"처럼 시간 기반 계산을 많이 하므로 중요합니다.

namespace rjm
{
    class Time
    {
    public:
        // Update:
        // - 이번 프레임에 걸린 시간을 저장하고, 전체 누적 시간에 더합니다.
        void Update(float deltaSeconds);

        // DeltaSeconds:
        // - 이번 프레임이 몇 초 걸렸는지 반환합니다.
        // - 예: 60FPS라면 대략 0.016초입니다.
        float DeltaSeconds() const;

        // TotalSeconds:
        // - 게임 시작 후 총 몇 초가 지났는지 반환합니다.
        float TotalSeconds() const;

    private:
        // 이번 프레임에 걸린 시간입니다.
        float deltaSeconds_ = 0.0f;

        // 게임 시작 이후 누적된 시간입니다.
        float totalSeconds_ = 0.0f;
    };
}

