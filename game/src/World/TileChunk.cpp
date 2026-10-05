// TileChunk.cpp
// - TileChunk의 위치 판정 함수 구현부입니다.

#include "RecoilJumpMan/World/TileChunk.h"

namespace rjm
{
    bool TileChunk::Contains(Vector2 worldPosition) const
    {
        return worldPosition.x >= bounds.x
            && worldPosition.x <= bounds.x + bounds.width
            && worldPosition.y >= bounds.y
            && worldPosition.y <= bounds.y + bounds.height;
    }

    bool TileChunk::Intersects(Rectangle worldRect) const
    {
        return bounds.x < worldRect.x + worldRect.width
            && bounds.x + bounds.width > worldRect.x
            && bounds.y < worldRect.y + worldRect.height
            && bounds.y + bounds.height > worldRect.y;
    }
}
