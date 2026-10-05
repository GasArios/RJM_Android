#pragma once

// Hud.h
// - HUD는 Heads-Up Display의 약자입니다.
// - 게임 화면 위에 체력, 탄약, 미니맵, 스킬 쿨타임 같은 정보를 표시하는 UI입니다.

namespace rjm
{
    // Player는 포인터/참조로만 사용하므로 전방 선언으로 충분합니다.
    class Player;

    class Hud
    {
    public:
        // Draw:
        // - 플레이어 상태를 읽어 HP와 탄약 정보를 화면에 그립니다.
        void Draw(const Player& player) const;
    };
}

