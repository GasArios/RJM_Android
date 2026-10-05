#pragma once

// Scene.h
// - Scene은 "게임의 한 화면 상태"를 표현하는 부모 클래스입니다.
// - 예: 타이틀 화면, 플레이 화면, 인벤토리 화면, 보스전 화면 등을 모두 Scene으로 만들 수 있습니다.
// - 이 파일은 모든 Scene이 공통으로 가져야 하는 함수 규칙을 정의합니다.

namespace rjm
{
    // GameContext는 여기서 포인터/참조로만 쓰이므로 전방 선언으로 충분합니다.
    struct GameContext;

    // Scene:
    // - 추상 클래스 역할을 합니다.
    // - 추상 클래스란 직접 객체로 만들기보다, 자식 클래스가 상속해서 쓰는 설계용 클래스입니다.
    class Scene
    {
    public:
        // virtual destructor:
        // - 부모 클래스 포인터로 자식 객체를 삭제할 때 자식 소멸자까지 안전하게 호출되도록 합니다.
        // - 상속을 염두에 둔 클래스에는 보통 virtual 소멸자를 둡니다.
        virtual ~Scene();

        // OnEnter:
        // - 이 씬으로 처음 들어올 때 호출됩니다.
        // - 기본 구현은 아무것도 하지 않지만, 자식 씬이 필요하면 override할 수 있습니다.
        virtual void OnEnter(GameContext& context);

        // OnExit:
        // - 이 씬에서 나갈 때 호출됩니다.
        // - 리소스 정리나 상태 저장이 필요할 때 사용할 수 있습니다.
        virtual void OnExit(GameContext& context);

        // Update:
        // - 매 프레임 게임 로직을 갱신합니다.
        // - = 0은 순수 가상 함수라는 뜻입니다.
        // - 자식 클래스가 반드시 직접 구현해야 합니다.
        virtual void Update(GameContext& context, float deltaSeconds) = 0;

        // Draw:
        // - 매 프레임 화면을 그립니다.
        // - const는 이 함수가 Scene 객체의 멤버 변수를 수정하지 않겠다는 약속입니다.
        virtual void Draw(GameContext& context) const = 0;
    };
}

