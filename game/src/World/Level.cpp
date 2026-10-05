// Level.cpp
// - Level은 현재 WorldMap을 감싸는 작은 facade입니다.

#include "RecoilJumpMan/World/Level.h"

namespace rjm
{
    Level::Level() = default;

    void Level::Update(float)
    {
    }

    void Level::Draw() const
    {
        world_.Draw();
    }

    Rectangle Level::WorldBounds() const
    {
        return world_.WorldBounds();
    }

    float Level::FloorWorldY() const
    {
        return world_.FloorWorldY();
    }

    WorldMap& Level::World()
    {
        return world_;
    }

    const WorldMap& Level::World() const
    {
        return world_;
    }
}
