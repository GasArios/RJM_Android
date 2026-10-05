#pragma once

// BuiltInData.h
// - 프로토타입/초기 데모에서 사용할 C++ 기반 데이터 카탈로그입니다.
// - 실제 수치는 .cpp에 두고, GameplayScene 같은 런타임 코드는 DataRegistry를 통해 id로 조회합니다.

#include "RecoilJumpMan/Data/EnemyDefinition.h"
#include "RecoilJumpMan/Data/WeaponDefinition.h"

#include <vector>

namespace rjm
{
    const std::vector<WeaponDefinition>& BuiltInWeaponDefinitions();
    const std::vector<EnemyDefinition>& BuiltInEnemyDefinitions();
}

