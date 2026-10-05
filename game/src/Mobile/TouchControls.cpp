#include "RecoilJumpMan/Mobile/TouchControls.h"
#include <algorithm>
#include <cmath>

namespace rjm::mobile {
TouchControls& Controls() { static TouchControls controls; return controls; }
bool TouchControls::Contains(Rectangle r, Vector2 p) {
    return p.x >= r.x && p.y >= r.y && p.x < r.x+r.width && p.y < r.y+r.height;
}
void TouchControls::BeginFrame() {
    if (!initialized_) { downIds_.fill(-1); initialized_ = true; }
    firePressed_ = aimChanged_ = false;
    selectedSlot_ = -1; action_ = Action::None;
}
void TouchControls::ChangeMenu(Menu menu) {
    menu_ = menu; fireId_ = braceId_ = -1;
    firePressed_ = aimChanged_ = false;
    fireBlocked_ = false;
    gateUntilRelease_ = std::any_of(downIds_.begin(), downIds_.end(), [](int id){ return id >= 0; });
}
void TouchControls::Back() {
    // Back from help leads to pause. A second Back resumes without firing.
    ChangeMenu(menu_ == Menu::Gameplay ? Menu::Pause : menu_ == Menu::Help ? Menu::Pause : Menu::Gameplay);
}
void TouchControls::Pause() { ChangeMenu(Menu::Pause); }
void TouchControls::BlockFireUntilRelease() { fireBlocked_ = true; firePressed_ = false; }
void TouchControls::Process(const TouchEvent& e) {
    if (!initialized_) BeginFrame();
    if (e.phase == TouchPhase::Cancel) {
        downIds_.fill(-1); fireId_ = braceId_ = -1;
        firePressed_ = aimChanged_ = false; gateUntilRelease_ = fireBlocked_ = false;
        return;
    }
    if (e.phase == TouchPhase::Up) {
        for (int& id : downIds_) if (id == e.id) id = -1;
        if (fireId_ == e.id) { fireId_ = -1; fireBlocked_ = false; }
        if (braceId_ == e.id) braceId_ = -1;
        if (std::all_of(downIds_.begin(), downIds_.end(), [](int id){ return id < 0; })) gateUntilRelease_ = false;
        return;
    }
    if (e.phase == TouchPhase::Move) {
        if (fireId_ == e.id && !fireBlocked_ && menu_ == Menu::Gameplay) {
            if (!e.inside) { fireBlocked_ = true; return; }
            const float dx = e.position.x-lastAimSample_.x, dy=e.position.y-lastAimSample_.y;
            // Ignore sensor-scale finger jitter; only intentional drag redirects held fire.
            if (dx*dx+dy*dy >= 100.0f) { aimPosition_=lastAimSample_=e.position; aimChanged_=true; }
        }
        return;
    }
    if (std::find(downIds_.begin(), downIds_.end(), e.id) != downIds_.end()) return;
    auto free = std::find(downIds_.begin(), downIds_.end(), -1);
    if (free == downIds_.end()) return;
    *free = e.id;
    if (gateUntilRelease_ || !e.inside) return;
    if (menu_ == Menu::Gameplay) {
        if (Contains(InfoButton, e.position)) { ChangeMenu(Menu::Help); return; }
        if (Contains(WeaponsButton, e.position)) { ChangeMenu(Menu::Weapons); return; }
        if (Contains(BraceButton, e.position)) { if (braceId_<0) braceId_=e.id; return; }
        if (fireId_<0) {
            fireId_=e.id; firePressed_=aimChanged_=true; fireBlocked_=false;
            aimPosition_=lastAimSample_=e.position;
        }
        return;
    }
    if (Contains(CloseButton, e.position)) { Back(); return; }
    if (menu_ == Menu::Weapons) {
        for(int slot=0;slot<3;++slot) if (Contains(WeaponButton(slot), e.position)) {
            selectedSlot_=slot; ChangeMenu(Menu::Gameplay); return;
        }
        if (Contains(ReloadButton,e.position)) { action_=Action::Reload; ChangeMenu(Menu::Gameplay); }
    } else if (menu_ == Menu::Pause) {
        if (Contains(ResumeButton,e.position)) ChangeMenu(Menu::Gameplay);
        else if (Contains(SaveButton,e.position)) action_=Action::Save;
        else if (Contains(RestartButton,e.position)) { action_=Action::Restart; ChangeMenu(Menu::Gameplay); }
        else if (Contains(AssistButton,e.position)) action_=Action::ToggleAssist;
        else if (Contains(ShakeButton,e.position)) action_=Action::ToggleShake;
    }
}
}
