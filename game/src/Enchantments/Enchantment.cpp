// Enchantment.cpp
// - Enchantment 클래스의 구현부입니다.
// - 현재는 생성자와 단순 getter 함수만 있습니다.

#include "RecoilJumpMan/Enchantments/Enchantment.h"

#include <utility>

namespace rjm
{
    // 생성자:
    // - std::move는 문자열을 복사하지 않고 내부 자원을 이동시키는 데 사용됩니다.
    Enchantment::Enchantment(DefinitionId id, std::string displayName, EnchantmentCategory category)
        : id_(std::move(id)), displayName_(std::move(displayName)), category_(category)
    {
    }

    // Id:
    // - id_를 const 참조로 반환합니다.
    const DefinitionId& Enchantment::Id() const
    {
        return id_;
    }

    // DisplayName:
    // - 표시 이름을 const 참조로 반환합니다.
    const std::string& Enchantment::DisplayName() const
    {
        return displayName_;
    }

    // Category:
    // - 카테고리 enum 값을 반환합니다.
    EnchantmentCategory Enchantment::Category() const
    {
        return category_;
    }
}

