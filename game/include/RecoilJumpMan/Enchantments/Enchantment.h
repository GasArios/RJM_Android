#pragma once

// Enchantment.h
// - 총기에 붙는 인챈트 정보를 표현하는 클래스입니다.
// - 현재는 id, 표시 이름, 카테고리만 가지고 있습니다.
// - 나중에는 공격력 증가, 반동 안정화, 공중 재장전 같은 효과 로직이 추가될 수 있습니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <string>

namespace rjm
{
    // EnchantmentCategory:
    // - 인챈트의 역할을 크게 나누는 분류입니다.
    enum class EnchantmentCategory
    {
        Offense,     // 공격력, 치명타, 관통 등 공격 관련입니다.
        Movement,    // 반동, 착지 보정 등 이동 관련입니다.
        Magazine,    // 탄창, 재장전, 탄 보존 관련입니다.
        Durability,  // 내구도, 수리, 고장 방지 관련입니다.
        Exploration  // 탐험, 지도, 봉인 파괴 등 특수 기능 관련입니다.
    };

    class Enchantment
    {
    public:
        // 생성자:
        // - id, 화면 표시 이름, 카테고리를 받아 인챈트 객체를 만듭니다.
        Enchantment(DefinitionId id, std::string displayName, EnchantmentCategory category);

        // Id:
        // - 인챈트 고유 id를 반환합니다.
        const DefinitionId& Id() const;

        // DisplayName:
        // - 화면에 보여줄 이름을 반환합니다.
        const std::string& DisplayName() const;

        // Category:
        // - 인챈트 카테고리를 반환합니다.
        EnchantmentCategory Category() const;

    private:
        // 데이터 식별용 id입니다.
        DefinitionId id_;

        // 화면 표시 이름입니다.
        std::string displayName_;

        // 인챈트 분류입니다.
        EnchantmentCategory category_;
    };
}

