#pragma once
#include <raylib.h>
#include <array>
#include "RecoilJumpMan/Mobile/AimPad.h"

namespace rjm::mobile {
enum class TouchPhase { Down, Move, Up, Cancel };
struct TouchEvent { TouchPhase phase; int id; Vector2 position; bool inside = true; };
enum class Menu { Gameplay, Weapons, Pause, Help };
enum class Action { None, Reload, Save, Restart, ToggleAssist, ToggleShake, ToggleAimMode, ResetZoom };

// UI keeps its physical scale when the world zooms. Panels are centered on the
// actual game area; corner buttons remain anchored to the left edge.
constexpr Rectangle InfoButton{24, 122, 88, 72};
constexpr Rectangle BraceButton{24, 380, 104, 92};
constexpr Rectangle LeftReloadButton{24, 484, 104, 56};
constexpr Rectangle WeaponsButton{24, 552, 104, 92};
Rectangle CloseButton();
Rectangle ResumeButton();
Rectangle SaveButton();
Rectangle RestartButton();
Rectangle AssistButton();
Rectangle ShakeButton();
Rectangle AimModeButton();
Rectangle ZoomResetButton();
Rectangle ReloadButton();
Rectangle WeaponButton(int slot);

class TouchControls {
public:
    void SetScreenSize(float width, float height);
    Vector2 ScreenSize() const { return screenSize_; }
    void SetAimMode(AimMode mode);
    AimMode CurrentAimMode() const {return aimMode_;}
    bool PadTouched() const {return IsPadMode(aimMode_) && fireId_>=0;}
    bool PadBlocked() const {return PadTouched() && fireBlocked_;}
    Vector2 PadKnob() const {return AimPad::Knob(screenSize_,aimPosition_,PadTouched());}
    void BeginFrame();
    void Process(const TouchEvent& event);
    void Back();
    void Pause();
    void BlockFireUntilRelease();
    Menu CurrentMenu() const { return menu_; }
    bool FirePressed() const { return firePressed_; }
    bool FireHeld() const { return fireId_ >= 0 && !fireBlocked_ && menu_ == Menu::Gameplay && !pinching_ && (!IsPadMode(aimMode_) || padActive_); }
    bool AimChanged() const { return aimChanged_; }
    bool Bracing() const { return braceId_ >= 0 && menu_ == Menu::Gameplay; }
    bool HasAim() const { return fireId_ >= 0 || firePressed_; }
    Vector2 AimPosition() const { return aimPosition_; }
    int SelectedSlot() const { return selectedSlot_; }
    Action RequestedAction() const { return action_; }
    bool Pinching() const { return pinching_; }
    float PinchScale() const { return pinchScale_; }
    float TimeScale() const {
        if(menu_ == Menu::Pause || menu_ == Menu::Help) return 0.0f;
        return menu_ == Menu::Weapons || pinching_ ? MobileTuning::SlowTimeScale : 1.0f;
    }
private:
    struct Pointer { int id = -1; Vector2 position{}; bool zoomCandidate = false; };
    void ChangeMenu(Menu menu);
    void UpdatePinch();
    void SamplePad(Vector2 point);
    bool IsUiPoint(Vector2 point) const;
    static bool Contains(Rectangle rect, Vector2 point);
    std::array<Pointer, 16> pointers_{};
    Vector2 screenSize_{1280,720};
    Menu menu_ = Menu::Gameplay;
    AimMode aimMode_ = AimMode::Character;
    bool padActive_ = false;
    int fireId_ = -1, braceId_ = -1;
    bool fireBlocked_ = false, gateUntilRelease_ = false;
    bool firePressed_ = false, aimChanged_ = false, pinching_ = false;
    Vector2 aimPosition_{}, lastAimSample_{};
    int pinchFirst_ = -1, pinchSecond_ = -1;
    float pinchDistance_ = 0, pinchScale_ = 1;
    int selectedSlot_ = -1;
    Action action_ = Action::None;
};
TouchControls& Controls();
}
