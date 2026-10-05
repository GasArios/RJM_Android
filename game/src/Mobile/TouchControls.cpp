#include "RecoilJumpMan/Mobile/TouchControls.h"
#include <algorithm>
#include <cmath>

namespace rjm::mobile {
TouchControls& Controls() { static TouchControls controls; return controls; }
namespace {
Rectangle Centered(float x,float y,float w,float h) {
    return {x+(Controls().ScreenSize().x-1280)*0.5f,y,w,h};
}
Rectangle PauseRow(int row) { return Centered(360,210+row*59.0f,560,50); }
}
Rectangle CloseButton() { return Centered(990,134,104,64); }
Rectangle ResumeButton() { return PauseRow(0); }
Rectangle SaveButton() { return PauseRow(1); }
Rectangle RestartButton() { return PauseRow(2); }
Rectangle AssistButton() { return PauseRow(3); }
Rectangle ShakeButton() { return PauseRow(4); }
Rectangle AimModeButton() { return PauseRow(5); }
Rectangle ZoomResetButton() { return PauseRow(6); }
Rectangle ReloadButton() { return Centered(400,510,480,64); }
Rectangle WeaponButton(int slot) { return Centered(250+slot*270.0f,260,240,196); }

bool TouchControls::Contains(Rectangle r, Vector2 p) {
    return p.x >= r.x && p.y >= r.y && p.x < r.x+r.width && p.y < r.y+r.height;
}
void TouchControls::SetScreenSize(float width,float height) {
    if(width==screenSize_.x && height==screenSize_.y) return;
    // A display-size change invalidates pointer coordinates and gesture spans.
    Process({TouchPhase::Cancel,-1,{}});
    screenSize_={width,height};
}
void TouchControls::BeginFrame() {
    firePressed_ = aimChanged_ = false;
    pinchScale_=1;
    selectedSlot_ = -1; action_ = Action::None;
}
void TouchControls::ChangeMenu(Menu menu) {
    menu_ = menu; fireId_ = braceId_ = -1;
    firePressed_ = aimChanged_ = false;
    fireBlocked_ = false; pinching_=false; pinchScale_=1;
    for(auto& p:pointers_) p.zoomCandidate=false;
    gateUntilRelease_ = std::any_of(pointers_.begin(),pointers_.end(),[](const Pointer& p){return p.id>=0;});
}
void TouchControls::Back() {
    ChangeMenu(menu_ == Menu::Gameplay ? Menu::Pause : menu_ == Menu::Help ? Menu::Pause : Menu::Gameplay);
}
void TouchControls::Pause() { ChangeMenu(Menu::Pause); }
void TouchControls::BlockFireUntilRelease() { fireBlocked_ = true; firePressed_ = false; }
bool TouchControls::IsUiPoint(Vector2 p) const {
    // Use this instance's width, including in isolated input tests.
    auto centered=[&](Rectangle r){r.x+=(screenSize_.x-1280)*0.5f; return r;};
    if(menu_==Menu::Gameplay) return Contains(InfoButton,p)||Contains(WeaponsButton,p)||Contains(BraceButton,p);
    if(Contains(centered({990,134,104,64}),p)) return true;
    if(menu_==Menu::Pause) for(int row=0;row<7;++row)
        if(Contains(centered({360,210+row*59.0f,560,50}),p)) return true;
    return menu_!=Menu::Pause;
}
void TouchControls::UpdatePinch() {
    const Pointer* first=nullptr; const Pointer* second=nullptr;
    for(const auto& p:pointers_) if(p.id>=0 && p.zoomCandidate) {
        if(!first) first=&p; else if(!second) second=&p;
    }
    if(!first || !second) return;
    const float distance=std::hypot(first->position.x-second->position.x,first->position.y-second->position.y);
    if(!pinching_) {
        pinching_=true; gateUntilRelease_=true;
        fireId_=-1; firePressed_=aimChanged_=false; fireBlocked_=true;
        pinchFirst_=first->id; pinchSecond_=second->id; pinchDistance_=distance;
        return;
    }
    if(first->id!=pinchFirst_ || second->id!=pinchSecond_) {
        pinchFirst_=first->id; pinchSecond_=second->id; pinchDistance_=distance; return;
    }
    if(distance>=MobileTuning::MinimumPinchSpan && pinchDistance_>=MobileTuning::MinimumPinchSpan)
        pinchScale_*=distance/pinchDistance_;
    pinchDistance_=distance;
}
void TouchControls::Process(const TouchEvent& e) {
    if(e.phase==TouchPhase::Cancel) {
        pointers_={}; fireId_=braceId_=-1;
        firePressed_=aimChanged_=false; gateUntilRelease_=fireBlocked_=pinching_=false;
        pinchScale_=1; pinchFirst_=pinchSecond_=-1;
        return;
    }
    if(e.phase==TouchPhase::Up) {
        for(auto& p:pointers_) if(p.id==e.id) p=Pointer{};
        if(fireId_==e.id) {fireId_=-1;fireBlocked_=false;}
        if(braceId_==e.id) braceId_=-1;
        if(std::all_of(pointers_.begin(),pointers_.end(),[](const Pointer& p){return p.id<0;})) {
            gateUntilRelease_=fireBlocked_=pinching_=false;
        }
        return;
    }
    if(e.phase==TouchPhase::Move) {
        for(auto& p:pointers_) if(p.id==e.id) { p.position=e.position; if(!e.inside) p.zoomCandidate=false; }
        if(pinching_) {UpdatePinch();return;}
        if(fireId_==e.id && !fireBlocked_ && menu_==Menu::Gameplay) {
            if(!e.inside) {fireBlocked_=true;return;}
            const float dx=e.position.x-lastAimSample_.x,dy=e.position.y-lastAimSample_.y;
            if(dx*dx+dy*dy>=MobileTuning::DragThreshold*MobileTuning::DragThreshold) {
                aimPosition_=lastAimSample_=e.position;aimChanged_=true;
            }
        }
        return;
    }
    if(std::any_of(pointers_.begin(),pointers_.end(),[&](const Pointer& p){return p.id==e.id;})) return;
    auto free=std::find_if(pointers_.begin(),pointers_.end(),[](const Pointer& p){return p.id<0;});
    if(free==pointers_.end()) return;
    *free={e.id,e.position,false};
    if(gateUntilRelease_ || !e.inside) return;
    free->zoomCandidate=(menu_==Menu::Gameplay || menu_==Menu::Pause) && !IsUiPoint(e.position);
    UpdatePinch();
    if(pinching_) return;
    const float shift=(screenSize_.x-1280)*0.5f;
    auto centered=[&](Rectangle r){r.x+=shift;return r;};
    if(menu_==Menu::Gameplay) {
        if(Contains(InfoButton,e.position)) {ChangeMenu(Menu::Help);return;}
        if(Contains(WeaponsButton,e.position)) {ChangeMenu(Menu::Weapons);return;}
        if(Contains(BraceButton,e.position)) {if(braceId_<0) braceId_=e.id;return;}
        if(fireId_<0) {
            fireId_=e.id;firePressed_=aimChanged_=true;fireBlocked_=false;
            aimPosition_=lastAimSample_=e.position;
        }
        return;
    }
    if(Contains(centered({990,134,104,64}),e.position)) {Back();return;}
    if(menu_==Menu::Weapons) {
        for(int slot=0;slot<3;++slot) if(Contains(centered({250+slot*270.0f,260,240,196}),e.position)) {
            selectedSlot_=slot;ChangeMenu(Menu::Gameplay);return;
        }
        if(Contains(centered({400,510,480,64}),e.position)) {action_=Action::Reload;ChangeMenu(Menu::Gameplay);}
    } else if(menu_==Menu::Pause) {
        int row=-1;
        for(int i=0;i<7;++i) if(Contains(centered({360,210+i*59.0f,560,50}),e.position)) row=i;
        switch(row) {
            case 0:ChangeMenu(Menu::Gameplay);break;
            case 1:action_=Action::Save;break;
            case 2:action_=Action::Restart;ChangeMenu(Menu::Gameplay);break;
            case 3:action_=Action::ToggleAssist;break;
            case 4:action_=Action::ToggleShake;break;
            case 5:action_=Action::ToggleAimMode;break;
            case 6:action_=Action::ResetZoom;break;
            default:break;
        }
    }
}
}
