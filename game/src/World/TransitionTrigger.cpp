// TransitionTrigger.cpp
// - 전환 트리거 판정 함수 구현부입니다.

#include "RecoilJumpMan/World/TransitionTrigger.h"

namespace rjm
{
    bool TransitionTrigger::Contains(Vector2 worldPosition) const
    {
        return worldPosition.x >= bounds.x
            && worldPosition.x <= bounds.x + bounds.width
            && worldPosition.y >= bounds.y
            && worldPosition.y <= bounds.y + bounds.height;
    }

    bool TransitionTrigger::RequiresInteraction() const
    {
        return kind == TransitionKind::Interact;
    }
}
