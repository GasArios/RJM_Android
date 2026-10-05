#pragma once

// SceneManager.h
// - 현재 실행 중인 Scene을 관리하는 클래스입니다.
// - 게임이 타이틀 화면에서 플레이 화면으로 넘어가거나,
//   게임 오버 화면으로 넘어가는 흐름을 이 클래스가 담당할 수 있습니다.

#include <memory>

namespace rjm
{
    struct GameContext;
    class Scene;

    class SceneManager
    {
    public:
        // 기본 생성자:
        // - SceneManager 객체가 만들어질 때 호출됩니다.
        SceneManager();

        // 소멸자:
        // - unique_ptr<Scene>이 불완전 타입 문제 없이 정리되도록 .cpp에서 구현합니다.
        ~SceneManager();

        // ChangeScene:
        // - 현재 씬을 종료하고 새 씬으로 교체합니다.
        // - std::unique_ptr<Scene>은 Scene 객체의 소유권을 단 하나의 포인터가 갖게 하는 스마트 포인터입니다.
        void ChangeScene(GameContext& context, std::unique_ptr<Scene> nextScene);

        // Clear:
        // - 현재 씬을 종료하고 제거합니다.
        void Clear(GameContext& context);

        // Update:
        // - 현재 씬이 있으면 그 씬의 Update를 호출합니다.
        void Update(GameContext& context, float deltaSeconds);

        // Draw:
        // - 현재 씬이 있으면 그 씬의 Draw를 호출합니다.
        void Draw(GameContext& context) const;

    private:
        // 현재 실행 중인 씬입니다.
        // unique_ptr이므로 SceneManager가 이 Scene 객체의 소유자입니다.
        std::unique_ptr<Scene> current_;
    };
}

