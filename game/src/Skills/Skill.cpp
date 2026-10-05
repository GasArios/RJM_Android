// Skill.cpp
// - Skill 클래스의 구현부입니다.
// - 현재는 값 저장과 getter 함수만 있습니다.

#include "RecoilJumpMan/Skills/Skill.h"

#include <utility>

namespace rjm
{
    // 생성자:
    // - 멤버 초기화 리스트로 모든 멤버를 초기화합니다.
    Skill::Skill(DefinitionId id, std::string displayName, SkillSlotKind slotKind, float cooldownSeconds)
        : id_(std::move(id)),
          displayName_(std::move(displayName)),
          slotKind_(slotKind),
          cooldownSeconds_(cooldownSeconds)
    {
    }

    // Id:
    // - 스킬 id를 반환합니다.
    const DefinitionId& Skill::Id() const
    {
        return id_;
    }

    // DisplayName:
    // - 스킬 이름을 반환합니다.
    const std::string& Skill::DisplayName() const
    {
        return displayName_;
    }

    // SlotKind:
    // - 스킬 슬롯 종류를 반환합니다.
    SkillSlotKind Skill::SlotKind() const
    {
        return slotKind_;
    }

    // CooldownSeconds:
    // - 스킬 쿨타임을 반환합니다.
    float Skill::CooldownSeconds() const
    {
        return cooldownSeconds_;
    }
}

