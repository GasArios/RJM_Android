// Item.cpp
// - Item 클래스의 구현부입니다.
// - 현재는 생성자와 getter 함수만 있습니다.

#include "RecoilJumpMan/Items/Item.h"

#include <utility>

namespace rjm
{
    // 생성자:
    // - id와 displayName은 문자열이므로 std::move로 멤버에 이동시킵니다.
    Item::Item(DefinitionId id, std::string displayName, ItemKind kind)
        : id_(std::move(id)), displayName_(std::move(displayName)), kind_(kind)
    {
    }

    // Id:
    // - 아이템 id를 반환합니다.
    const DefinitionId& Item::Id() const
    {
        return id_;
    }

    // DisplayName:
    // - 아이템 이름을 반환합니다.
    const std::string& Item::DisplayName() const
    {
        return displayName_;
    }

    // Kind:
    // - 아이템 종류를 반환합니다.
    ItemKind Item::Kind() const
    {
        return kind_;
    }
}

