#pragma once

// InputState.h
// - 키보드와 마우스 입력 상태를 한 프레임 단위로 모아두는 클래스입니다.
// - Raylib의 IsKeyPressed 같은 함수를 게임 곳곳에서 직접 부르면 코드가 흩어집니다.
// - 그래서 입력을 이 클래스에서 한 번 읽고, 다른 시스템은 InputState를 통해 확인합니다.

#include <raylib.h>

namespace rjm
{
    class ViewportScaler;

    class InputState
    {
    public:
        // Poll:
        // - 현재 프레임의 키보드/마우스 입력 상태를 Raylib에서 읽어옵니다.
        // - viewport를 사용해 실제 창 좌표의 마우스를 1280x720 가상 화면 좌표로 변환합니다.
        void Poll(const ViewportScaler& viewport);
        bool AimChanged() const { return aimChanged_; }

        // MousePosition:
        // - 현재 마우스 좌표를 가상 화면 좌표로 반환합니다.
        // - 레터박스가 있을 때도 게임 로직은 이 좌표를 사용해야 조준이 어긋나지 않습니다.
        // - Vector2는 Raylib가 제공하는 2D 벡터 구조체입니다. x, y 값을 가집니다.
        Vector2 MousePosition() const;

        // WindowMousePosition:
        // - 실제 창 기준 마우스 좌표를 반환합니다.
        // - 레터박스 영역 위에 있는지 확인하거나, 창 전체 UI를 만들 때 사용할 수 있습니다.
        Vector2 WindowMousePosition() const;

        // MouseInsideGameViewport:
        // - 마우스가 실제 창 안의 16:9 게임 화면 영역 위에 있는지 반환합니다.
        // - false면 검은 바/장식 영역 위에 있는 것이므로 조준과 발사를 막는 것이 자연스럽습니다.
        bool MouseInsideGameViewport() const;

        // MouseWheelMove:
        // - 이번 프레임의 마우스 휠 움직임을 반환합니다.
        // - 양수는 보통 위로 스크롤, 음수는 아래로 스크롤입니다.
        float MouseWheelMove() const;

        // FirePressed:
        // - 좌클릭이 "이번 프레임에 막 눌렸는지" 반환합니다.
        bool FirePressed() const;

        // FireHeld:
        // - 좌클릭을 "누르고 있는 중인지" 반환합니다.
        bool FireHeld() const;

        // AimLockPressed:
        // - 우클릭이 "이번 프레임에 막 눌렸는지" 반환합니다.
        // - 지금은 우클릭 홀드 락온을 쓰지만, 나중에 토글 옵션을 만들 때 사용할 수 있습니다.
        bool AimLockPressed() const;

        // AimLockHeld:
        // - 우클릭을 "누르고 있는 중인지" 반환합니다.
        // - 전투용 자동 락온 모드는 이 값이 true인 동안만 켜집니다.
        bool AimLockHeld() const;

        // ReloadPressed:
        // - R 키가 이번 프레임에 눌렸는지 반환합니다.
        bool ReloadPressed() const;

        // FireModeTogglePressed:
        // - B 키가 이번 프레임에 눌렸는지 반환합니다.
        // - 발사 모드가 여러 개인 무기에서 Semi/Auto 전환에 사용합니다.
        bool FireModeTogglePressed() const;

        // InteractPressed:
        // - E 키가 이번 프레임에 눌렸는지 반환합니다.
        // - 포탈, 문, NPC 대화 같은 상호작용에 사용합니다.
        bool InteractPressed() const;

        // BraceHeld:
        // - 버티기 키를 누르고 있는지 반환합니다.
        bool BraceHeld() const;

        // SelectedWeaponSlot:
        // - 이번 프레임에 선택한 무기 슬롯 번호를 반환합니다.
        // - 아무 슬롯도 선택하지 않았다면 -1을 반환합니다.
        int SelectedWeaponSlot() const;

    private:
        bool aimChanged_ = false;
        // 현재 마우스 위치입니다.
        // 게임이 사용하는 1280x720 가상 화면 좌표입니다.
        Vector2 mousePosition_ = { 0.0f, 0.0f };

        // 실제 창 기준 마우스 위치입니다.
        Vector2 windowMousePosition_ = { 0.0f, 0.0f };

        // 마우스가 게임 화면 viewport 안에 있는지 저장합니다.
        bool mouseInsideGameViewport_ = true;

        // 이번 프레임의 마우스 휠 움직임입니다.
        float mouseWheelMove_ = 0.0f;

        // 좌클릭이 이번 프레임에 눌렸는지 저장합니다.
        bool firePressed_ = false;

        // 좌클릭을 누르고 있는지 저장합니다.
        bool fireHeld_ = false;

        // 우클릭이 이번 프레임에 눌렸는지 저장합니다.
        bool aimLockPressed_ = false;

        // 우클릭을 누르고 있는지 저장합니다.
        bool aimLockHeld_ = false;

        // R 키가 이번 프레임에 눌렸는지 저장합니다.
        bool reloadPressed_ = false;

        // B 키가 이번 프레임에 눌렸는지 저장합니다.
        bool fireModeTogglePressed_ = false;

        // E 키가 이번 프레임에 눌렸는지 저장합니다.
        bool interactPressed_ = false;

        // Ctrl 또는 S를 누르고 있는지 저장합니다.
        bool braceHeld_ = false;

        // Z, X, C로 선택한 무기 슬롯입니다. 선택이 없으면 -1입니다.
        int selectedWeaponSlot_ = -1;
    };
}
