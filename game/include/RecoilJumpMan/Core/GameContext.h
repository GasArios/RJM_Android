#pragma once

// GameContext.h
// - 여러 시스템이 공통으로 필요로 하는 객체의 주소를 묶어 전달하는 구조체입니다.
// - 예를 들어 GameplayScene은 입력, 시간, 에셋에 접근해야 할 수 있습니다.
// - 매번 여러 개의 인자를 따로 넘기기보다 GameContext 하나로 묶어 넘깁니다.

namespace rjm
{
    // 전방 선언:
    // - class 이름만 미리 알려주는 C++ 문법입니다.
    // - 포인터나 참조로만 사용할 때는 전체 class 정의를 몰라도 됩니다.
    // - 이렇게 하면 불필요한 include를 줄일 수 있습니다.
    class AssetManager;
    class DataRegistry;
    class InputState;
    class Time;
    class ViewportScaler;

    // struct:
    // - class와 거의 같지만, 기본 접근 제한이 public입니다.
    // - 단순히 데이터를 묶을 때 자주 씁니다.
    struct GameContext
    {
        // Time 객체의 주소입니다.
        // nullptr는 아직 아무것도 가리키지 않는 포인터 값입니다.
        Time* time = nullptr;
        const ViewportScaler* viewport = nullptr;

        // InputState 객체의 주소입니다.
        InputState* input = nullptr;

        // AssetManager 객체의 주소입니다.
        AssetManager* assets = nullptr;

        // DataRegistry 객체의 주소입니다.
        // 총기, 적, 아이템 같은 데이터 정의를 id로 찾을 때 사용합니다.
        DataRegistry* data = nullptr;
    };
}

