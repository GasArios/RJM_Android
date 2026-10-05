// Area.cpp
// - Area 관련 간단한 판정 함수 구현부입니다.

#include "RecoilJumpMan/World/Area.h"

namespace rjm
{
    bool Area::Contains(Vector2 worldPosition) const
    {
        return worldPosition.x >= bounds.x
            && worldPosition.x <= bounds.x + bounds.width
            && worldPosition.y >= bounds.y
            && worldPosition.y <= bounds.y + bounds.height;
    }
}
