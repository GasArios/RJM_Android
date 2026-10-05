// InputState.cpp
// - InputState 클래스의 구현부입니다.
// - Raylib 입력 함수를 호출해 현재 프레임 입력 상태를 저장합니다.

#include "RecoilJumpMan/Core/InputState.h"

#include "RecoilJumpMan/Core/ViewportScaler.h"
#include "RecoilJumpMan/Mobile/Platform.h"
#include "RecoilJumpMan/Mobile/TouchControls.h"

namespace rjm
{
    // Poll:
    // - 매 프레임 한 번 호출되어 입력 상태를 갱신합니다.
    void InputState::Poll(const ViewportScaler& viewport)
    {
#ifdef __ANDROID__
        auto& controls = mobile::Controls();
        controls.BeginFrame();
        auto events = mobile::DrainTouchEvents();
        if (mobile::ConsumePause()) {
            controls.Process({mobile::TouchPhase::Cancel, -1, {}});
            controls.Pause();
            events.clear();
            mobile::ConsumeBack();
        } else if (mobile::ConsumeBack()) controls.Back();
        for (auto event : events) {
            event.inside = viewport.ContainsWindowPoint(event.position);
            event.position = viewport.WindowToVirtual(event.position);
            controls.Process(event);
        }
        mousePosition_ = controls.AimPosition();
        mouseInsideGameViewport_ = controls.HasAim() && controls.CurrentMenu() == mobile::Menu::Gameplay;
        firePressed_ = controls.FirePressed(); fireHeld_ = controls.FireHeld();
        aimChanged_ = controls.AimChanged(); braceHeld_ = controls.Bracing();
        selectedWeaponSlot_ = controls.SelectedSlot();
        reloadPressed_ = controls.RequestedAction() == mobile::Action::Reload;
        mouseWheelMove_ = 0.0f;
        aimLockPressed_ = aimLockHeld_ = fireModeTogglePressed_ = interactPressed_ = false;
        return;
#endif
        // GetMousePosition은 현재 마우스 좌표를 실제 창 기준으로 반환하는 Raylib 함수입니다.
        windowMousePosition_ = GetMousePosition();

        // 실제 창 좌표를 게임의 가상 화면 좌표로 변환합니다.
        // 예: 창이 1000x1000이어도 게임 로직은 1280x720 기준 마우스 좌표를 받습니다.
        mousePosition_ = viewport.WindowToVirtual(windowMousePosition_);

        // 레터박스/필러박스 영역 위에 마우스가 있는지 확인합니다.
        mouseInsideGameViewport_ = viewport.ContainsWindowPoint(windowMousePosition_);

        // GetMouseWheelMove는 이번 프레임의 마우스 휠 이동량을 반환합니다.
        mouseWheelMove_ = GetMouseWheelMove();

        // IsMouseButtonPressed는 이번 프레임에 버튼이 눌린 순간만 true입니다.
        firePressed_ = IsMouseButtonPressed(MOUSE_BUTTON_LEFT);

        // IsMouseButtonDown은 버튼을 누르고 있는 동안 계속 true입니다.
        fireHeld_ = IsMouseButtonDown(MOUSE_BUTTON_LEFT);

        // 우클릭은 전투용 자동 락온 모드 입력으로 사용합니다.
        // Pressed는 나중에 토글 옵션을 만들 때 쓸 수 있고, Held는 현재 홀드 방식 락온에 사용합니다.
        aimLockPressed_ = IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
        aimLockHeld_ = IsMouseButtonDown(MOUSE_BUTTON_RIGHT);

        // R 키가 이번 프레임에 눌렸는지 확인합니다.
        reloadPressed_ = IsKeyPressed(KEY_R);

        // B 키가 이번 프레임에 눌렸는지 확인합니다.
        // 배틀그라운드식 발사 모드 전환 키로 사용합니다.
        fireModeTogglePressed_ = IsKeyPressed(KEY_B);

        // E 키가 이번 프레임에 눌렸는지 확인합니다.
        interactPressed_ = IsKeyPressed(KEY_E);

        // ||는 논리 OR입니다.
        // 왼쪽이나 오른쪽 중 하나만 true여도 전체가 true가 됩니다.
        braceHeld_ = IsKeyDown(KEY_LEFT_CONTROL) || IsKeyDown(KEY_S);

        // 매 프레임 기본값을 -1로 초기화합니다.
        // 그래야 이번 프레임에 아무 슬롯 키도 안 눌렀다는 뜻을 명확히 표현할 수 있습니다.
        selectedWeaponSlot_ = -1;

        // if / else if:
        // - 위 조건이 true면 아래 조건은 검사하지 않습니다.
        // - 한 프레임에 하나의 슬롯 선택만 처리하려는 의도입니다.
        if (IsKeyPressed(KEY_Z))
        {
            selectedWeaponSlot_ = 0;
        }
        else if (IsKeyPressed(KEY_X))
        {
            selectedWeaponSlot_ = 1;
        }
        else if (IsKeyPressed(KEY_C))
        {
            selectedWeaponSlot_ = 2;
        }
    }

    // MousePosition:
    // - 저장된 마우스 좌표를 반환합니다.
    Vector2 InputState::MousePosition() const
    {
        return mousePosition_;
    }

    // WindowMousePosition:
    // - 실제 창 좌표를 반환합니다.
    Vector2 InputState::WindowMousePosition() const
    {
        return windowMousePosition_;
    }

    // MouseInsideGameViewport:
    // - 마우스가 16:9 게임 화면 영역 안에 있으면 true입니다.
    bool InputState::MouseInsideGameViewport() const
    {
        return mouseInsideGameViewport_;
    }

    // MouseWheelMove:
    // - 저장된 마우스 휠 이동량을 반환합니다.
    float InputState::MouseWheelMove() const
    {
        return mouseWheelMove_;
    }

    // FirePressed:
    // - 발사 버튼이 이번 프레임에 눌렸는지 반환합니다.
    bool InputState::FirePressed() const
    {
        return firePressed_;
    }

    // FireHeld:
    // - 발사 버튼을 누르고 있는지 반환합니다.
    bool InputState::FireHeld() const
    {
        return fireHeld_;
    }

    bool InputState::AimLockPressed() const
    {
        return aimLockPressed_;
    }

    bool InputState::AimLockHeld() const
    {
        return aimLockHeld_;
    }

    // ReloadPressed:
    // - 재장전 키가 눌렸는지 반환합니다.
    bool InputState::ReloadPressed() const
    {
        return reloadPressed_;
    }

    // FireModeTogglePressed:
    // - 발사 모드 전환 키가 눌렸는지 반환합니다.
    bool InputState::FireModeTogglePressed() const
    {
        return fireModeTogglePressed_;
    }

    // InteractPressed:
    // - 상호작용 키가 눌렸는지 반환합니다.
    bool InputState::InteractPressed() const
    {
        return interactPressed_;
    }

    // BraceHeld:
    // - 버티기 키를 누르고 있는지 반환합니다.
    bool InputState::BraceHeld() const
    {
        return braceHeld_;
    }

    // SelectedWeaponSlot:
    // - 선택된 무기 슬롯 번호를 반환합니다.
    int InputState::SelectedWeaponSlot() const
    {
        return selectedWeaponSlot_;
    }
}
