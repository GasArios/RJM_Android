#pragma once

// ProjectileHitPayload.h
// - 투사체가 충돌했을 때 전달할 전투 정보를 담습니다.
// - Damage 하나만 들고 있으면 관통탄, 폭발탄, 전용 피격 이펙트 같은 확장이 어려워집니다.
// - 그래서 피해량, 명중 피드백, 이펙트 id, 관통/폭발 규칙을 한 묶음으로 둡니다.

#include "RecoilJumpMan/Combat/Damage.h"
#include "RecoilJumpMan/Data/DefinitionId.h"
#include "RecoilJumpMan/Data/WeaponDefinition.h"

namespace rjm
{
    struct ProjectileHitPayload
    {
        // 적이나 오브젝트에 전달할 피해 정보입니다.
        Damage damage;

        // 이 투사체가 유효한 대상에게 명중했을 때 사용할 피드백입니다.
        // 발사 피드백과 명중 피드백을 분리해, "쏘는 맛"과 "맞히는 맛"을 따로 조절할 수 있습니다.
        WeaponFeedbackProfile feedback;

        // 피격 이펙트를 고르는 데이터 id입니다.
        // 현재는 기본 파티클만 쓰지만, 나중에는 스프라이트 파티클/사운드/흔적 이펙트를 이 id로 고를 수 있습니다.
        DefinitionId hitEffectId = "default_hit";

        // 첫 명중 이후 추가로 관통할 수 있는 대상 수입니다.
        // 0이면 일반 탄처럼 첫 대상에 맞고 사라집니다.
        int pierceCount = 0;

        // 폭발탄이나 범위 피해 탄환을 위한 예약 값입니다.
        // 0 이하이면 범위 피해가 없는 일반 투사체로 취급합니다.
        float explosionRadius = 0.0f;

        // true이면 명중 후 관통 수를 확인하고 투사체를 제거합니다.
        // false이면 빔, 지속 장판, 다단 히트 영역처럼 맞아도 사라지지 않는 투사체를 만들 수 있습니다.
        bool destroyOnHit = true;
    };
}

