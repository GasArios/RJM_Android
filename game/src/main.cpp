// 이 파일은 C++ 프로그램의 시작점입니다.
// C/C++ 프로그램은 반드시 main() 함수에서 실행을 시작합니다.

#include "RecoilJumpMan/Core/Application.h"

// main 함수:
// - 운영체제가 프로그램을 실행할 때 가장 먼저 호출하는 함수입니다.
// - int를 반환하는 이유는 프로그램이 정상 종료되었는지 숫자로 알려주기 위해서입니다.
// - 보통 0을 반환하면 정상 종료, 0이 아니면 오류 종료로 봅니다.
int main()
{
    // rjm은 Recoil Jump Man 코드를 담는 namespace입니다.
    // namespace는 이름 충돌을 막기 위한 C++ 문법입니다.
    //
    // Application 객체는 Raylib 창 생성, 게임 루프 실행, 창 종료를 담당합니다.
    rjm::Application app;

    // app.Run()을 호출하면 실제 게임 프로그램이 돌아갑니다.
    // Run()의 반환값을 그대로 main의 반환값으로 사용합니다.
    return app.Run();
}
