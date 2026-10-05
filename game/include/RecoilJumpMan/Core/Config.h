#pragma once

// Config.h
// - 게임 전체에서 공통으로 쓰는 설정값을 모아두는 파일입니다.
// - 창 크기, 목표 FPS, 게임 제목, 에셋 폴더 경로처럼 자주 참조되는 값을 둡니다.
// - inline constexpr는 C++17 문법입니다.
//   constexpr: 컴파일 시점에 값이 정해지는 상수라는 뜻입니다.
//   inline: 헤더 파일에 정의해도 중복 정의 오류가 나지 않도록 해줍니다.

namespace rjm::config
{
    // 게임이 내부적으로 그리는 가상 화면의 가로 픽셀 크기입니다.
    // 실제 창 크기가 바뀌어도 게임 로직, 카메라, HUD는 이 크기를 기준으로 동작합니다.
    inline constexpr int VirtualWidth = 1280;

    // 게임이 내부적으로 그리는 가상 화면의 세로 픽셀 크기입니다.
    // 1280x720은 16:9 비율입니다.
    inline constexpr int VirtualHeight = 720;

    // 프로그램을 처음 열 때 사용할 창의 가로 픽셀 크기입니다.
    // 플레이어는 이후 창 테두리를 드래그해 자유롭게 크기를 바꿀 수 있습니다.
    inline constexpr int InitialWindowWidth = VirtualWidth;

    // 프로그램을 처음 열 때 사용할 창의 세로 픽셀 크기입니다.
    inline constexpr int InitialWindowHeight = VirtualHeight;

    // 창 크기를 아주 작게 줄이면 조작과 UI 판독이 어려워지므로 최소 크기를 둡니다.
    inline constexpr int MinimumWindowWidth = 640;
    inline constexpr int MinimumWindowHeight = 360;

    // Raylib가 초당 몇 프레임을 목표로 게임 루프를 돌릴지 정합니다.
    inline constexpr int TargetFps = 60;

    // 창 제목 표시줄에 보이는 게임 이름입니다.
    inline constexpr const char* GameTitle = "Recoil Jump Man";

    // 이미지, 사운드, 데이터 파일을 찾을 때 기준이 되는 폴더입니다.
    inline constexpr const char* AssetRoot = "assets/";
}

