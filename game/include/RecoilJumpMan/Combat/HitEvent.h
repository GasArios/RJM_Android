#pragma once

// HitEvent.h
// - 전투 판정 결과로 발생한 "명중 사건"을 표현합니다.
// - 투사체 충돌 코드는 HitEvent만 만들고,
//   렌더링, 사운드, 카메라 피드백, 퀘스트 시스템은 이 이벤트를 보고 각자 반응합니다.
// - 이렇게 하면 피격 이펙트를 바꿔도 충돌/피해 코드를 건드리지 않아도 됩니다.

#include "RecoilJumpMan/Combat/Damage.h"
#include "RecoilJumpMan/Data/DefinitionId.h"
#include "RecoilJumpMan/Data/WeaponDefinition.h"

#include <raylib.h>

namespace rjm
{
    struct HitEvent
    {
        // 공격을 발생시킨 데이터 id입니다.
        // 현재는 무기 id를 넣고 있지만, 나중에는 스킬/함정/적 탄환 id도 들어갈 수 있습니다.
        DefinitionId sourceId;

        // 피해를 받은 대상의 데이터 id입니다.
        // 적 정의 id, 보스 id, 파괴 가능한 오브젝트 id 등에 사용할 수 있습니다.
        DefinitionId targetId;

        // 피격 이펙트를 고르는 데이터 id입니다.
        // 현재 HitEffectSystem은 기본 파티클로 처리하지만, 나중에 이 id로 전용 이펙트를 고를 수 있습니다.
        DefinitionId hitEffectId = "default_hit";

        // 명중이 발생한 월드 좌표입니다.
        // 피격 파티클, 데미지 숫자, 사운드 위치 같은 연출의 기준점입니다.
        Vector2 position = { 0.0f, 0.0f };

        // 피격 방향입니다.
        // 예: 총알이 적의 오른쪽에서 맞았다면 파티클을 오른쪽으로 더 튀게 할 수 있습니다.
        Vector2 normal = { 0.0f, 1.0f };

        // 요청된 피해 정보입니다.
        Damage damage;

        // 대상이 실제로 피해를 받은 뒤의 결과입니다.
        // 방어력/무적/약점 보정이 들어가면 damage.amount와 appliedAmount가 달라질 수 있습니다.
        DamageResult damageResult;

        // 명중 순간에 발생시킬 히트스톱/카메라 흔들림 데이터입니다.
        WeaponFeedbackProfile feedback;
    };
}

