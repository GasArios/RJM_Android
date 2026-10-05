#pragma once

// Application.h
// - Application 클래스의 선언부입니다.
// - "선언"은 이런 클래스와 함수가 존재한다고 컴파일러에게 알려주는 역할입니다.
// - 실제 함수 내용은 src/Core/Application.cpp에 있습니다.

namespace rjm
{
    // Application:
    // - 프로그램 전체의 가장 바깥 껍데기입니다.
    // - Raylib 창을 열고, 게임 루프를 실행하고, 종료 시 정리합니다.
    class Application
    {
    public:
        // Run:
        // - 게임 프로그램을 실제로 실행합니다.
        // - int를 반환해서 main 함수에 종료 코드를 전달합니다.
        int Run();
    };
}

