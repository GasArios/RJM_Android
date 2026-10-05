// Application.cpp
// - Application 클래스 함수들의 실제 구현부입니다.
// - Raylib 초기화와 종료, 그리고 매 프레임 반복되는 게임 루프가 여기에 있습니다.

#include "RecoilJumpMan/Core/Application.h"

#include "RecoilJumpMan/Core/Config.h"
#include "RecoilJumpMan/Core/Game.h"
#include "RecoilJumpMan/Core/ViewportScaler.h"

#include <raylib.h>

namespace rjm
{
    // Application::Run:
    // - 클래스 바깥에서 멤버 함수를 구현할 때는 ClassName::FunctionName 형태를 씁니다.
    // - 여기서는 Application 클래스의 Run 함수를 구현하고 있습니다.
    int Application::Run()
    {
        // SetConfigFlags는 InitWindow 전에 호출해야 하는 Raylib 설정 함수입니다.
        // FLAG_WINDOW_RESIZABLE을 켜면 플레이어가 창 테두리를 드래그해 창 크기를 바꿀 수 있습니다.
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);

        // InitWindow는 Raylib 함수입니다.
        // 게임 창을 만들고 그래픽 시스템을 초기화합니다.
#ifdef __ANDROID__
        InitWindow(0, 0, config::GameTitle);
        SetExitKey(KEY_NULL);
#else
        InitWindow(config::InitialWindowWidth, config::InitialWindowHeight, config::GameTitle);
#endif

        // 창을 너무 작게 줄이면 UI와 조작이 읽기 어려우므로 최소 크기를 지정합니다.
        SetWindowMinSize(config::MinimumWindowWidth, config::MinimumWindowHeight);

        // SetTargetFPS는 Raylib 함수입니다.
        // 게임 루프가 초당 60번 정도 돌도록 목표 프레임을 설정합니다.
        SetTargetFPS(config::TargetFps);

        // Game 객체는 게임 내부 시스템을 묶어둔 중심 객체입니다.
        Game game;

        // ViewportScaler:
        // - 실제 창 크기가 얼마든 게임 화면은 config::VirtualWidth x config::VirtualHeight에 그립니다.
        // - 이후 실제 창에는 16:9 비율을 유지한 채 확대/축소해서 출력합니다.
        ViewportScaler viewport(config::VirtualWidth, config::VirtualHeight);

        // RenderTexture:
        // - 게임 월드와 HUD를 직접 창에 그리지 않고, 먼저 고정 가상 해상도 텍스처에 그립니다.
        // - 그 다음 텍스처를 실제 창 크기에 맞춰 비율 유지로 출력합니다.
        RenderTexture2D gameTarget = LoadRenderTexture(config::VirtualWidth, config::VirtualHeight);
        SetTextureFilter(gameTarget.texture, TEXTURE_FILTER_POINT);

        // 게임 시작 시 필요한 초기화를 수행합니다.
        game.Initialize();

        // WindowShouldClose는 창 닫기 버튼이나 ESC 입력 등을 확인하는 Raylib 함수입니다.
        // false인 동안 계속 반복하므로, 이 while문이 게임의 메인 루프입니다.
        while (!WindowShouldClose())
        {
            // 실제 창 크기는 플레이어가 매 프레임 바꿀 수 있으므로 계속 다시 읽습니다.
            viewport.Update(GetScreenWidth(), GetScreenHeight());

            // GetFrameTime은 지난 프레임에서 이번 프레임까지 걸린 시간을 초 단위로 반환합니다.
            // 이 값을 사용하면 컴퓨터 성능이 달라도 이동 속도를 일정하게 만들 수 있습니다.
            game.Update(GetFrameTime(), viewport);

            // 먼저 게임을 고정 가상 해상도 RenderTexture에 그립니다.
            // 이 안에서는 카메라와 HUD가 항상 1280x720 기준으로 동작합니다.
            BeginTextureMode(gameTarget);
            game.Draw();
            EndTextureMode();

            // BeginDrawing과 EndDrawing 사이에 있는 Draw 함수들이 한 프레임 화면을 그립니다.
            BeginDrawing();

            // 실제 창에는 레터박스/필러박스 장식 영역을 먼저 그리고,
            // 그 위에 16:9 비율로 맞춘 게임 텍스처를 출력합니다.
            viewport.DrawLetterboxDecorations();
            viewport.DrawGameTexture(gameTarget);

            EndDrawing();
        }

        // 게임에서 사용한 리소스를 정리합니다.
        game.Shutdown();

        // RenderTexture는 Raylib 리소스이므로 창을 닫기 전에 해제합니다.
        UnloadRenderTexture(gameTarget);

        // Raylib 창과 그래픽 시스템을 종료합니다.
        CloseWindow();

        // 0은 정상 종료를 뜻합니다.
        return 0;
    }
}
