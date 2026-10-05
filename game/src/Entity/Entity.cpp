// Entity.cpp
// - Entity 기본 클래스의 구현부입니다.
// - 대부분의 함수는 단순히 멤버 값을 반환하거나 설정합니다.

#include "RecoilJumpMan/Entity/Entity.h"

namespace rjm
{
    // 기본 소멸자 구현입니다.
    Entity::~Entity() = default;

    // Update 기본 구현:
    // - 부모 Entity 자체는 특별한 동작이 없습니다.
    // - 자식 클래스가 필요하면 override해서 사용합니다.
    void Entity::Update(float)
    {
    }

    // Draw 기본 구현:
    // - 부모 Entity 자체는 그릴 모양이 없습니다.
    void Entity::Draw() const
    {
    }

    // Position:
    // - 현재 위치를 반환합니다.
    Vector2 Entity::Position() const
    {
        return position_;
    }

    // Velocity:
    // - 현재 속도를 반환합니다.
    Vector2 Entity::Velocity() const
    {
        return velocity_;
    }

    // IsActive:
    // - active_ 값을 반환합니다.
    bool Entity::IsActive() const
    {
        return active_;
    }

    // SetPosition:
    // - 외부에서 엔티티 위치를 설정할 때 사용합니다.
    void Entity::SetPosition(Vector2 position)
    {
        position_ = position;
    }

    // SetVelocity:
    // - 외부에서 엔티티 속도를 설정할 때 사용합니다.
    void Entity::SetVelocity(Vector2 velocity)
    {
        velocity_ = velocity;
    }

    // SetActive:
    // - 엔티티 활성 상태를 변경합니다.
    void Entity::SetActive(bool active)
    {
        active_ = active;
    }
}

