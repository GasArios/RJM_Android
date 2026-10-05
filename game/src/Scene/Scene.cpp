// Scene.cpp
// - Scene 부모 클래스의 기본 함수 구현부입니다.
// - OnEnter와 OnExit는 기본적으로 아무 일도 하지 않도록 비워둡니다.

#include "RecoilJumpMan/Scene/Scene.h"

namespace rjm
{
    // = default:
    // - 컴파일러가 만들어주는 기본 소멸자 구현을 사용하겠다는 뜻입니다.
    // - 헤더에 선언만 하고 여기서 구현하는 이유는 가상 소멸자의 실제 정의가 필요하기 때문입니다.
    Scene::~Scene() = default;

    // OnEnter 기본 구현:
    // - 부모 클래스에서는 할 일이 없습니다.
    // - 매개변수 이름을 생략하면 "받기는 하지만 사용하지 않는다"는 뜻을 표현할 수 있습니다.
    void Scene::OnEnter(GameContext&)
    {
    }

    // OnExit 기본 구현:
    // - 부모 클래스에서는 할 일이 없습니다.
    void Scene::OnExit(GameContext&)
    {
    }
}

