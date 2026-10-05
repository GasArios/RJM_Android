#pragma once
#include <raylib.h>
#include <array>

namespace rjm::mobile {
enum class TouchPhase { Down, Move, Up, Cancel };
struct TouchEvent { TouchPhase phase; int id; Vector2 position; bool inside = true; };
enum class Menu { Gameplay, Weapons, Pause, Help };
enum class Action { None, Reload, Save, Restart, ToggleAssist, ToggleShake };

// All geometry is in the same 1280x720 virtual coordinates used for drawing.
constexpr Rectangle InfoButton{24, 122, 88, 72};
constexpr Rectangle BraceButton{24, 380, 104, 92};
constexpr Rectangle WeaponsButton{24, 552, 104, 92};
constexpr Rectangle CloseButton{990, 134, 104, 64};
constexpr Rectangle ResumeButton{380, 226, 520, 60};
constexpr Rectangle SaveButton{380, 298, 520, 60};
constexpr Rectangle RestartButton{380, 370, 520, 60};
constexpr Rectangle AssistButton{380, 442, 520, 60};
constexpr Rectangle ShakeButton{380, 514, 520, 60};
constexpr Rectangle ReloadButton{400, 510, 480, 64};
constexpr Rectangle WeaponButton(int slot) { return {250.0f + slot * 270.0f, 260, 240, 196}; }

class TouchControls {
public:
    void BeginFrame();
    void Process(const TouchEvent& event);
    void Back();
    void Pause();
    void BlockFireUntilRelease();
    Menu CurrentMenu() const { return menu_; }
    bool FirePressed() const { return firePressed_; }
    bool FireHeld() const { return fireId_ >= 0 && !fireBlocked_ && menu_ == Menu::Gameplay; }
    bool AimChanged() const { return aimChanged_; }
    bool Bracing() const { return braceId_ >= 0 && menu_ == Menu::Gameplay; }
    bool HasAim() const { return fireId_ >= 0 || firePressed_; }
    Vector2 AimPosition() const { return aimPosition_; }
    int SelectedSlot() const { return selectedSlot_; }
    Action RequestedAction() const { return action_; }
    float TimeScale() const { return menu_ == Menu::Weapons ? 0.08f : menu_ == Menu::Gameplay ? 1.0f : 0.0f; }
private:
    void ChangeMenu(Menu menu);
    static bool Contains(Rectangle rect, Vector2 point);
    std::array<int, 16> downIds_{};
    bool initialized_ = false;
    Menu menu_ = Menu::Gameplay;
    int fireId_ = -1, braceId_ = -1;
    bool fireBlocked_ = false, gateUntilRelease_ = false;
    bool firePressed_ = false, aimChanged_ = false;
    Vector2 aimPosition_{}, lastAimSample_{};
    int selectedSlot_ = -1;
    Action action_ = Action::None;
};
TouchControls& Controls();
}
