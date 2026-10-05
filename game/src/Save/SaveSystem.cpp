// SaveSystem.cpp
// - SaveSystem 클래스의 구현부입니다.
// - 현재는 텍스트 파일에 줄 단위로 저장하고 다시 읽습니다.

#include "RecoilJumpMan/Save/SaveSystem.h"

#include <fstream>

namespace rjm
{
    // Save:
    // - std::ofstream은 파일에 쓰기 위한 출력 스트림입니다.
    bool SaveSystem::Save(const SaveData& data, const std::string& path) const
    {
        std::ofstream file(path);
        if (!file)
        {
            return false;
        }

        // << 연산자로 파일에 값을 씁니다.
        // '\n'은 줄바꿈 문자입니다.
        file << data.currentLevelId << '\n';
        file << data.playerX << ' ' << data.playerY << '\n';
        file << data.maxCarryWeight << '\n';
        return true;
    }

    // Load:
    // - std::ifstream은 파일에서 읽기 위한 입력 스트림입니다.
    bool SaveSystem::Load(const std::string& path, SaveData& outData) const
    {
        std::ifstream file(path);
        if (!file)
        {
            return false;
        }

        // >> 연산자로 파일에서 값을 읽어옵니다.
        file >> outData.currentLevelId;
        file >> outData.playerX >> outData.playerY;
        file >> outData.maxCarryWeight;
        return true;
    }
}

