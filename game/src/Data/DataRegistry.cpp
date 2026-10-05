#include "RecoilJumpMan/Data/DataRegistry.h"

#include "RecoilJumpMan/Data/BuiltInData.h"

namespace rjm
{
    bool DataRegistry::LoadBuiltIns()
    {
        Clear();

        for (const WeaponDefinition& weapon : BuiltInWeaponDefinitions())
        {
            weapons_[weapon.id] = weapon;
        }

        for (const EnemyDefinition& enemy : BuiltInEnemyDefinitions())
        {
            enemies_[enemy.id] = enemy;
        }

        return true;
    }

    const WeaponDefinition* DataRegistry::FindWeapon(const DefinitionId& id) const
    {
        const auto found = weapons_.find(id);
        return found == weapons_.end() ? nullptr : &found->second;
    }

    const EnemyDefinition* DataRegistry::FindEnemy(const DefinitionId& id) const
    {
        const auto found = enemies_.find(id);
        return found == enemies_.end() ? nullptr : &found->second;
    }

    void DataRegistry::Clear()
    {
        weapons_.clear();
        enemies_.clear();
    }
}

