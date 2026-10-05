// DebugOverlay.cpp
// - DebugOverlay 클래스의 구현부입니다.
// - Raylib의 GetFPS와 Time 정보를 이용해 화면 아래에 디버그 텍스트를 표시합니다.

#include "RecoilJumpMan/Debug/DebugOverlay.h"

#include "RecoilJumpMan/Core/GameContext.h"
#include "RecoilJumpMan/Core/Time.h"

#include <raylib.h>

#include <cstdio>

namespace rjm
{
    // Draw:
    // - context.time이 있으면 누적 시간을 읽고, 없으면 0으로 표시합니다.
    void DebugOverlay::Draw(const GameContext& context) const
    {
        char text[96];

        // 포인터가 nullptr일 수 있으므로 삼항 연산자로 안전하게 처리합니다.
        const float totalSeconds = context.time ? context.time->TotalSeconds() : 0.0f;

        std::snprintf(text, sizeof(text), "FPS %d | Time %.1f", GetFPS(), totalSeconds);
        DrawText(text, 20, 684, 18, Color{ 150, 165, 190, 255 });
    }
}

