#pragma once

// SaveData.h
// - 저장 파일에 기록할 게임 상태를 담는 구조체입니다.
// - 현재는 매우 작은 시작 버전입니다.

#include <string>

namespace rjm
{
    struct SaveData
    {
        // 현재 플레이어가 있는 레벨 id입니다.
        std::string currentLevelId;

        // 플레이어의 저장된 x 좌표입니다.
        float playerX = 0.0f;

        // 플레이어의 저장된 y 좌표입니다.
        float playerY = 0.0f;

        // 플레이어가 장착 가능한 총 무게 한도입니다.
        int maxCarryWeight = 6;
    };
}

