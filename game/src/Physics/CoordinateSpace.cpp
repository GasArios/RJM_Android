// CoordinateSpace.cpp
// - 물리 월드 좌표와 Raylib 렌더 좌표를 변환하는 함수들의 구현부입니다.

#include "RecoilJumpMan/Physics/CoordinateSpace.h"

namespace rjm
{
    Vector2 CoordinateSpace::WorldToRender(Vector2 worldPosition)
    {
        return { worldPosition.x, -worldPosition.y };
    }

    Vector2 CoordinateSpace::RenderToWorld(Vector2 renderPosition)
    {
        return { renderPosition.x, -renderPosition.y };
    }

    Vector2 CoordinateSpace::WorldDirectionToRender(Vector2 worldDirection)
    {
        return { worldDirection.x, -worldDirection.y };
    }

    Vector2 CoordinateSpace::RenderDirectionToWorld(Vector2 renderDirection)
    {
        return { renderDirection.x, -renderDirection.y };
    }
}

