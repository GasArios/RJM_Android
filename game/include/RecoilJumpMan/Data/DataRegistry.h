#pragma once

// DataRegistry.h
// - 게임 데이터 정의를 id로 찾아주는 중앙 저장소입니다.
// - 로더가 JSON이든 C++ built-in 데이터든, GameplayScene은 이 Registry만 보도록 경계를 둡니다.

#include "RecoilJumpMan/Data/DefinitionId.h"
#include "RecoilJumpMan/Data/EnemyDefinition.h"
#include "RecoilJumpMan/Data/WeaponDefinition.h"

#include <unordered_map>

namespace rjm
{
    class DataRegistry
    {
    public:
        // LoadBuiltIns:
        // - BuiltInData.cpp에 정의된 C++ 데이터들을 Registry에 등록합니다.
        // - 나중에 파일 기반 데이터가 필요해지면 이 함수 옆에 LoadFromFiles 같은 입구를 추가하면 됩니다.
        bool LoadBuiltIns();

        const WeaponDefinition* FindWeapon(const DefinitionId& id) const;
        const EnemyDefinition* FindEnemy(const DefinitionId& id) const;

        void Clear();

    private:
        std::unordered_map<DefinitionId, WeaponDefinition> weapons_;
        std::unordered_map<DefinitionId, EnemyDefinition> enemies_;
    };
}

