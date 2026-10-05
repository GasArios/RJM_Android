#pragma once

// EnemyDefinition.h
// - 일반 몬스터나 보스의 기본 데이터를 담는 구조입니다.
// - 실제 AI 로직이 아니라 체력, 접촉 피해, 이동 속도 같은 수치 정보를 담습니다.

#include "RecoilJumpMan/Data/DefinitionId.h"
#include "RecoilJumpMan/Entity/EnemyAi.h"

#include <string>

namespace rjm
{
    // EnemyRank:
    // - 적의 등급을 구분합니다.
    enum class EnemyRank
    {
        Normal, // 일반 몬스터입니다.
        Elite,  // 강화 몬스터입니다.
        Boss    // 보스입니다.
    };

    struct EnemyDefinition
    {
        // 적 데이터의 고유 id입니다.
        DefinitionId id;

        // 화면에 표시할 이름입니다.
        std::string displayName;

        // 적의 등급입니다.
        EnemyRank rank = EnemyRank::Normal;

        // 최대 체력입니다.
        float maxHealth = 1.0f;

        // 플레이어와 닿았을 때 주는 피해입니다.
        float contactDamage = 1.0f;

        // 기본 이동 속도입니다.
        float moveSpeed = 0.0f;

        // 적 몸체의 한 변 길이입니다.
        // 현재 일반 적은 정사각형 AABB 하나로 충돌/피격/디버그 렌더링을 공유합니다.
        float bodySize = 28.0f;

        // crowdSeparationEnabled:
        // - true이면 같은 층의 다른 적과 너무 많이 겹쳤을 때 부드럽게 밀려납니다.
        // - 유령, 환영, 완전히 겹쳐도 되는 소환체는 false로 둘 수 있습니다.
        bool crowdSeparationEnabled = true;

        // crowdSeparationAllowedOverlapRatio:
        // - 몸 크기 대비 이 비율만큼의 가로 겹침은 허용합니다.
        // - 0이면 거의 겹치지 않고, 값이 클수록 무리 지어 보이는 느낌을 허용합니다.
        float crowdSeparationAllowedOverlapRatio = 0.25f;

        // crowdSeparationStrength:
        // - 겹침을 한 프레임에 얼마나 적극적으로 풀지 결정합니다.
        float crowdSeparationStrength = 0.65f;

        // crowdSeparationMaxPush:
        // - 군중 분리로 한 프레임에 밀릴 수 있는 최대 거리입니다.
        float crowdSeparationMaxPush = 6.0f;

        // crowdSeparationVerticalOverlapRatio:
        // - y축 겹침이 이 비율보다 작으면 다른 층/다른 발판으로 보고 분리하지 않습니다.
        float crowdSeparationVerticalOverlapRatio = 0.45f;

        // 일반 필드 몬스터용 공통 AI 성향입니다.
        // 별도 AI 클래스를 몬스터마다 만들지 않고, 이 프로필 값 조합으로 행동 차이를 냅니다.
        EnemyAiProfile ai;

        // aimAssistEnabled:
        // - 이 적이 조준 보정 후보로 들어갈 수 있는지 나타냅니다.
        // - 은신 적, 환영, 무적 상태 보스 페이즈처럼 보정을 걸면 안 되는 대상은 false로 둘 수 있습니다.
        bool aimAssistEnabled = true;

        // aimAssistRadiusPixels:
        // - 마우스 포인터가 이 적 근처에 있다고 볼 화면 픽셀 반경입니다.
        // - 0 이하이면 무기의 기본 보정 반경을 사용합니다.
        float aimAssistRadiusPixels = 0.0f;

        // aimAssistPriority:
        // - 같은 거리라면 priority가 높은 후보가 더 쉽게 선택됩니다.
        // - 보스 약점, 큰 적의 중심점, 튜토리얼 타깃처럼 의도적으로 잘 잡히게 할 대상에 사용할 수 있습니다.
        float aimAssistPriority = 1.0f;
    };
}

