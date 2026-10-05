#pragma once

// SaveSystem.h
// - SaveData를 파일로 저장하고, 파일에서 다시 읽어오는 클래스입니다.
// - 현재는 아주 단순한 텍스트 저장 방식입니다.

#include "RecoilJumpMan/Save/SaveData.h"

#include <string>

namespace rjm
{
    class SaveSystem
    {
    public:
        // Save:
        // - SaveData를 path 위치의 파일에 저장합니다.
        // - 성공하면 true, 실패하면 false를 반환합니다.
        bool Save(const SaveData& data, const std::string& path) const;

        // Load:
        // - path 위치의 파일에서 SaveData를 읽어 outData에 채웁니다.
        // - 성공하면 true, 실패하면 false를 반환합니다.
        bool Load(const std::string& path, SaveData& outData) const;
    };
}

