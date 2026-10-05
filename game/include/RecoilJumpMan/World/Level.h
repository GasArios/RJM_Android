#pragma once

// Level.h
// - 현재 플레이 중인 월드 인스턴스의 얇은 진입점입니다.
// - 실제 거대한 맵 데이터는 WorldMap이 들고 있고, Level은 GameplayScene이 접근하기 쉬운 facade 역할을 합니다.

#include "RecoilJumpMan/World/WorldMap.h"

#include <raylib.h>

namespace rjm
{
    class Level
    {
    public:
        Level();

        void Update(float deltaSeconds);
        void Draw() const;

        Rectangle WorldBounds() const;
        float FloorWorldY() const;

        WorldMap& World();
        const WorldMap& World() const;

    private:
        WorldMap world_;
    };
}

