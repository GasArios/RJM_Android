#pragma once

// Skill.h
// - 플레이어가 장착하고 사용하는 스킬 정보를 표현하는 클래스입니다.
// - 현재는 id, 이름, 슬롯 종류, 쿨타임만 가지고 있습니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <string>

namespace rjm
{
    // SkillSlotKind:
    // - 스킬이 어느 종류의 슬롯에 들어가는지 구분합니다.
    enum class SkillSlotKind
    {
        Combat,   // 공격/전투 스킬입니다.
        Utility,  // 보조/유틸리티 스킬입니다.
        Movement  // 이동 전용 스킬입니다.
    };

    class Skill
    {
    public:
        // 생성자:
        // - 스킬의 기본 정보를 받아 객체를 만듭니다.
        Skill(DefinitionId id, std::string displayName, SkillSlotKind slotKind, float cooldownSeconds);

        // Id:
        // - 스킬 고유 id를 반환합니다.
        const DefinitionId& Id() const;

        // DisplayName:
        // - 화면 표시 이름을 반환합니다.
        const std::string& DisplayName() const;

        // SlotKind:
        // - 이 스킬이 전투/유틸/이동 중 어디에 속하는지 반환합니다.
        SkillSlotKind SlotKind() const;

        // CooldownSeconds:
        // - 스킬 사용 후 다시 쓰기까지 걸리는 시간을 반환합니다.
        float CooldownSeconds() const;

    private:
        // 데이터 식별용 id입니다.
        DefinitionId id_;

        // 화면 표시 이름입니다.
        std::string displayName_;

        // 스킬 슬롯 종류입니다.
        SkillSlotKind slotKind_;

        // 쿨타임 시간입니다.
        float cooldownSeconds_ = 0.0f;
    };
}

