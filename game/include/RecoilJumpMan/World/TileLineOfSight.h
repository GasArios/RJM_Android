#pragma once

// TileLineOfSight.h
// - TileMap의 solid 타일을 기준으로 두 월드 좌표 사이의 시야선을 샘플링합니다.

#include <raylib.h>

namespace rjm
{
    class TileMap;

    struct TileLineOfSightResult
    {
        bool hasLineOfSight = true;
        Vector2 visibleEndWorld = { 0.0f, 0.0f };
    };

    class TileLineOfSight
    {
    public:
        // Trace:
        // - fromWorld에서 toWorld까지 선분을 따라 solid 타일을 샘플링합니다.
        // - 막히면 hasLineOfSight=false와 마지막으로 열려 있던 좌표를 반환합니다.
        static TileLineOfSightResult Trace(const TileMap& tileMap, Vector2 fromWorld, Vector2 toWorld);
    };
}

