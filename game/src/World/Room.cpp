// Room.cpp
// - Room 클래스의 구현부입니다.
// - 현재는 데이터 저장과 getter 함수만 있습니다.

#include "RecoilJumpMan/World/Room.h"

#include <utility>

namespace rjm
{
    // 생성자:
    // - id는 문자열이므로 std::move로 멤버에 이동시킵니다.
    Room::Room(std::string id, Rectangle bounds, TraversalTier traversalTier)
        : id_(std::move(id)), bounds_(bounds), traversalTier_(traversalTier)
    {
    }

    // Id:
    // - 방 id를 반환합니다.
    const std::string& Room::Id() const
    {
        return id_;
    }

    // Bounds:
    // - 방 영역을 반환합니다.
    Rectangle Room::Bounds() const
    {
        return bounds_;
    }

    // MinimumTraversalTier:
    // - 방의 이동 난이도를 반환합니다.
    TraversalTier Room::MinimumTraversalTier() const
    {
        return traversalTier_;
    }
}
