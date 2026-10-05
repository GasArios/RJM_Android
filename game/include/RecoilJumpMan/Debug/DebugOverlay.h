#pragma once

// DebugOverlay.h
// - 개발 중 확인용 정보를 화면에 표시하는 클래스입니다.
// - 현재는 FPS와 게임 누적 시간을 표시합니다.

namespace rjm
{
    struct GameContext;

    class DebugOverlay
    {
    public:
        // Draw:
        // - GameContext에서 시간 정보를 읽어 디버그 텍스트를 그립니다.
        void Draw(const GameContext& context) const;
    };
}

