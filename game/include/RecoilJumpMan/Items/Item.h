#pragma once

// Item.h
// - 소모품이나 유틸리티 아이템의 기본 정보를 표현하는 클래스입니다.
// - 현재는 id, 이름, 종류만 가지고 있습니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <string>

namespace rjm
{
    // ItemKind:
    // - 아이템의 역할을 구분합니다.
    enum class ItemKind
    {
        Healing,     // 회복 물약 같은 회복 아이템입니다.
        Defense,     // 방어막 캡슐 같은 방어 아이템입니다.
        Repair,      // 수리 키트 같은 장비 수리 아이템입니다.
        Movement,    // 순간 착지추, 반동 증폭제 같은 이동 보조 아이템입니다.
        Exploration, // 균열 탐지기, 귀환 표식 같은 탐험 아이템입니다.
        Utility      // 그 외 보조 아이템입니다.
    };

    class Item
    {
    public:
        // 생성자:
        // - 아이템 기본 정보를 받아 객체를 만듭니다.
        Item(DefinitionId id, std::string displayName, ItemKind kind);

        // Id:
        // - 아이템 고유 id를 반환합니다.
        const DefinitionId& Id() const;

        // DisplayName:
        // - 화면 표시 이름을 반환합니다.
        const std::string& DisplayName() const;

        // Kind:
        // - 아이템 종류를 반환합니다.
        ItemKind Kind() const;

    private:
        // 데이터 식별용 id입니다.
        DefinitionId id_;

        // 화면 표시 이름입니다.
        std::string displayName_;

        // 아이템 종류입니다.
        ItemKind kind_;
    };
}

