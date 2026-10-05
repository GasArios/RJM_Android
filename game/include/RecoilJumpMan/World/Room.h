#pragma once

// Room.h
// - 레벨 안의 방/구역 하나를 표현하는 클래스입니다.
// - 메트로베니아 구조에서는 레벨 하나가 여러 방으로 나뉘는 경우가 많습니다.

#include "RecoilJumpMan/Data/LevelDefinition.h"

#include <raylib.h>
#include <string>

namespace rjm
{
    class Room
    {
    public:
        // 기본 생성자:
        // - vector 같은 컨테이너에서 기본 생성이 필요할 수 있어 둡니다.
        Room() = default;

        // 생성자:
        // - 방 id, 영역 사각형, 필요한 이동 등급을 받아 Room을 만듭니다.
        Room(std::string id, Rectangle bounds, TraversalTier traversalTier);

        // Id:
        // - 방 고유 id를 반환합니다.
        const std::string& Id() const;

        // Bounds:
        // - 방이 차지하는 사각형 영역을 반환합니다.
        Rectangle Bounds() const;

        // MinimumTraversalTier:
        // - 이 방에 접근하거나 통과하기 위한 이동 난이도를 반환합니다.
        TraversalTier MinimumTraversalTier() const;

    private:
        // 방의 고유 id입니다.
        std::string id_;

        // 방의 위치와 크기입니다.
        Rectangle bounds_ = { 0.0f, 0.0f, 0.0f, 0.0f };

        // 이 방의 최소 이동 난이도입니다.
        TraversalTier traversalTier_ = TraversalTier::T0;
    };
}

