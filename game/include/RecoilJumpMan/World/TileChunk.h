#pragma once

// TileChunk.h
// - Chunk는 로딩/언로딩 단위가 아니라 빠른 조회를 위한 공간 분할 단위입니다.
// - 맵은 통째로 메모리에 올려두되, 렌더/충돌/스폰 후보 조회는 주변 chunk만 보도록 만들 수 있습니다.

#include <raylib.h>

#include <cstddef>
#include <vector>

namespace rjm
{
    struct ChunkCoord
    {
        int x = 0;
        int y = 0;
    };

    struct TileChunk
    {
        ChunkCoord coord;
        Rectangle bounds = { 0.0f, 0.0f, 0.0f, 0.0f };
        std::vector<std::size_t> spawnPointIndices;

        bool Contains(Vector2 worldPosition) const;
        bool Intersects(Rectangle worldRect) const;
    };
}

