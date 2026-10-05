#pragma once

// Game.h
// - Game 클래스의 선언부입니다.
// - Game은 시간, 입력, 에셋, 씬을 묶어 관리하는 "게임 본체" 역할입니다.

#include "RecoilJumpMan/Assets/AssetManager.h"
#include "RecoilJumpMan/Core/GameContext.h"
#include "RecoilJumpMan/Core/InputState.h"
#include "RecoilJumpMan/Core/Time.h"
#include "RecoilJumpMan/Core/ViewportScaler.h"
#include "RecoilJumpMan/Data/DataRegistry.h"
#include "RecoilJumpMan/Scene/SceneManager.h"

namespace rjm
{
    // Game:
    // - Application보다 한 단계 안쪽에 있는 게임 시스템 관리자입니다.
    // - Application은 창과 루프를 담당하고, Game은 실제 게임 상태를 담당합니다.
    class Game
    {
    public:
        // 생성자:
        // - 객체가 만들어질 때 자동으로 호출되는 특별한 함수입니다.
        // - 여기서는 GameContext가 내부 시스템들을 가리키도록 연결합니다.
        Game();

        // Initialize:
        // - 게임 시작 시 한 번 호출됩니다.
        // - 현재는 GameplayScene으로 진입합니다.
        void Initialize();

        // Shutdown:
        // - 게임 종료 시 한 번 호출됩니다.
        // - 씬과 에셋을 정리합니다.
        void Shutdown();

        // Update:
        // - 매 프레임 호출됩니다.
        // - deltaTime은 지난 프레임 이후 흐른 시간입니다.
        // - viewport는 실제 창 좌표를 가상 화면 좌표로 변환하기 위해 입력 시스템에 전달됩니다.
        void Update(float deltaTime, const ViewportScaler& viewport);

        // Draw:
        // - 매 프레임 호출됩니다.
        // - 현재 씬의 화면을 그립니다.
        void Draw();

    private:
        // Time 객체:
        // - 이번 프레임 시간과 전체 누적 시간을 관리합니다.
        Time time_;

        // InputState 객체:
        // - 마우스, 키보드 입력 상태를 한곳에 모읍니다.
        InputState input_;

        // AssetManager 객체:
        // - 이미지 같은 Raylib 리소스를 로드하고 해제합니다.
        AssetManager assets_;

        // DataRegistry 객체:
        // - C++ built-in 데이터 정의를 id로 조회할 수 있게 보관합니다.
        DataRegistry data_;

        // SceneManager 객체:
        // - 현재 실행 중인 씬을 관리합니다.
        SceneManager scenes_;

        // GameContext 객체:
        // - 여러 시스템이 공통으로 필요한 포인터들을 묶어 전달합니다.
        GameContext context_;
    };
}

