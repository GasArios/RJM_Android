// Game.cpp
// - Game 클래스의 구현부입니다.
// - 시간 갱신, 입력 갱신, 씬 업데이트/그리기 같은 중심 흐름이 있습니다.

#include "RecoilJumpMan/Core/Game.h"

#include "RecoilJumpMan/Scene/GameplayScene.h"

#include <memory>

namespace rjm
{
    // Game 생성자:
    // - 생성자는 반환형을 쓰지 않습니다.
    // - Game 객체가 만들어질 때 context_가 내부 객체들을 가리키도록 연결합니다.
    Game::Game()
    {
        // &는 주소 연산자입니다.
        // time_ 객체 자체를 복사하는 것이 아니라, time_이 있는 메모리 주소를 context_에 저장합니다.
        context_.time = &time_;
        context_.input = &input_;
        context_.assets = &assets_;
        context_.data = &data_;
    }

    // Initialize:
    // - 게임이 처음 시작될 때 호출됩니다.
    // - std::make_unique<GameplayScene>()는 GameplayScene 객체를 동적 생성하고 unique_ptr로 감쌉니다.
    void Game::Initialize()
    {
        data_.LoadBuiltIns();
        scenes_.ChangeScene(context_, std::make_unique<GameplayScene>());
    }

    // Shutdown:
    // - 게임 종료 직전에 호출됩니다.
    // - 현재 씬을 제거하고, 로드된 에셋을 해제합니다.
    void Game::Shutdown()
    {
        scenes_.Clear(context_);
        assets_.UnloadAll();
    }

    // Update:
    // - 매 프레임 게임 상태를 갱신합니다.
    // - 순서는 보통 시간 갱신 -> 입력 갱신 -> 게임 로직 갱신입니다.
    void Game::Update(float deltaTime, const ViewportScaler& viewport)
    {
        time_.Update(deltaTime);
        input_.Poll(viewport);
        scenes_.Update(context_, time_.DeltaSeconds());
    }

    // Draw:
    // - 매 프레임 화면을 그립니다.
    // - 실제 그리기는 현재 씬에게 위임합니다.
    void Game::Draw()
    {
        scenes_.Draw(context_);
    }
}

