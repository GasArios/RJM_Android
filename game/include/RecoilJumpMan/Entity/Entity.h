#pragma once

// Entity.h
// - 게임 안에 존재하는 "물체"의 가장 기본이 되는 클래스입니다.
// - 플레이어, 적, 총알, 아이템 드롭 등은 모두 위치와 속도를 가질 수 있습니다.
// - position_과 velocity_는 게임 내부 물리 좌표계를 기준으로 저장합니다.
//   즉 y가 커질수록 위로 올라갑니다. Raylib 화면 좌표와 반대입니다.
// - 기본 규칙:
//   Entity::Position()은 엔티티의 중심 좌표입니다.
//   왼쪽 위 좌표나 바닥 좌표가 필요하면 각 엔티티가 Draw/충돌 계산 시점에 파생해서 만듭니다.
// - 그런 공통점을 Entity에 모아둡니다.

#include <raylib.h>

namespace rjm
{
    class Entity
    {
    public:
        // virtual 소멸자:
        // - Entity를 상속한 자식 객체를 Entity 포인터로 삭제해도 안전하게 합니다.
        virtual ~Entity();

        // Update:
        // - 매 프레임 이 엔티티의 상태를 갱신합니다.
        // - virtual이므로 자식 클래스에서 자기 방식으로 재정의할 수 있습니다.
        virtual void Update(float deltaSeconds);

        // Draw:
        // - 매 프레임 이 엔티티를 화면에 그립니다.
        virtual void Draw() const;

        // Position:
        // - 현재 위치를 반환합니다.
        // - 기본 규칙상 이 값은 엔티티의 중심 월드 좌표입니다.
        Vector2 Position() const;

        // Velocity:
        // - 현재 속도를 반환합니다.
        Vector2 Velocity() const;

        // IsActive:
        // - 이 엔티티가 아직 살아있는지, 업데이트/그리기 대상인지 반환합니다.
        bool IsActive() const;

        // SetPosition:
        // - 위치를 새 값으로 바꿉니다.
        void SetPosition(Vector2 position);

        // SetVelocity:
        // - 속도를 새 값으로 바꿉니다.
        void SetVelocity(Vector2 velocity);

        // SetActive:
        // - 엔티티 활성 상태를 바꿉니다.
        // - 예를 들어 총알 수명이 끝나면 false로 만들 수 있습니다.
        void SetActive(bool active);

    protected:
        // protected:
        // - private와 public의 중간입니다.
        // - 외부에서는 접근할 수 없지만, 자식 클래스는 직접 접근할 수 있습니다.

        // 엔티티의 현재 위치입니다.
        // 게임 내부 물리 좌표계 기준입니다. y+는 위쪽입니다.
        // 기본 규칙상 중심 좌표로 저장합니다.
        Vector2 position_ = { 0.0f, 0.0f };

        // 엔티티의 현재 속도입니다.
        // 게임 내부 물리 좌표계 기준입니다. y+는 상승 속도입니다.
        Vector2 velocity_ = { 0.0f, 0.0f };

        // false면 풀이나 컬렉션이 업데이트/그리기에서 제외하거나 제거할 수 있습니다.
        bool active_ = true;
    };
}

