#include "RecoilJumpMan/Scene/GameplayScene.h"
#include "RecoilJumpMan/Core/GameContext.h"
#include "RecoilJumpMan/Core/InputState.h"
#include "RecoilJumpMan/Mobile/Platform.h"
#include "RecoilJumpMan/Mobile/TouchControls.h"
#include <fstream>
#include <cstdio>
#include <cmath>
#include <algorithm>

namespace {
void Button(Rectangle r,const char* text,bool selected=false) {
    DrawRectangleRounded(r,0.13f,8,selected?Color{35,96,148,240}:Color{26,40,59,238});
    DrawRectangleLinesEx(r,2,selected?Color{110,203,255,255}:Color{88,119,150,255});
    DrawText(text,static_cast<int>(r.x+(r.width-MeasureText(text,22))/2),static_cast<int>(r.y+(r.height-22)/2),22,RAYWHITE);
}
std::string CheckpointPath() { return rjm::mobile::StoragePath()+"/checkpoint-v1.txt"; }
}
namespace rjm {
bool GameplayScene::SaveMobileCheckpoint() const {
    const Vector2 position=ResolveRecoveryPosition();
    const std::string path=CheckpointPath(), temp=path+".tmp";
    std::ofstream file(temp,std::ios::trunc);
    if(!file) return false;
    // Test-field checkpoint, not a full RPG world save. Always restart grounded
    // at the last confirmed safe position with restored health and magazines.
    file << "RJM_CHECKPOINT_1\n" << position.x << ' ' << position.y << ' '
         << player_.Weapons().CurrentSlot() << ' ' << mobileAssist_ << ' ' << mobileShake_ << '\n';
    file.flush(); if(!file) return false;
    file.close();
    return std::rename(temp.c_str(),path.c_str())==0;
}
void GameplayScene::LoadMobileCheckpoint() {
    std::ifstream file(CheckpointPath());
    std::string version; Vector2 position{}; int slot=0, assist=1, shake=1;
    if(!(file>>version>>position.x>>position.y>>slot>>assist>>shake) || version!="RJM_CHECKPOINT_1") return;
    const Rectangle bounds=level_.WorldBounds(); const float half=player_.HalfBodySize();
    if(!std::isfinite(position.x) || !std::isfinite(position.y) || slot<0 || slot>=3 ||
        position.x<bounds.x+half || position.x>bounds.x+bounds.width-half ||
        position.y<bounds.y+half || position.y>bounds.y+bounds.height-half) return;
    const auto& map=level_.World().Map();
    for(float x:{-half+1,half-1}) for(float y:{-half+1,half-1})
        if(map.IsSolidAtWorld({position.x+x,position.y+y})) return;
    player_.SetPosition(position); player_.SetVelocity({});
    player_.Weapons().Select(slot); lastSafePosition_=position;
    mobileAssist_=assist!=0; mobileShake_=shake!=0;
}
bool GameplayScene::UpdateMobile(GameContext& context,float deltaSeconds) {
    using namespace mobile;
    const auto& controls=Controls();
    const int menu=static_cast<int>(controls.CurrentMenu());
    if(menu!=mobilePreviousMenu_) {
        TraceLog(LOG_INFO,"RJM: menu=%d",menu);
        if(controls.CurrentMenu()==Menu::Pause) SaveMobileCheckpoint();
        mobilePreviousMenu_=menu;
    }
    switch(controls.RequestedAction()) {
        case Action::Save: mobileNotice_=SaveMobileCheckpoint()?"Safe checkpoint saved":"Save failed"; break;
        case Action::ToggleAssist: mobileAssist_=!mobileAssist_; SaveMobileCheckpoint(); break;
        case Action::ToggleShake: mobileShake_=!mobileShake_; SaveMobileCheckpoint(); break;
        case Action::Restart:
            std::remove(CheckpointPath().c_str());
            player_=Player{}; mobileReload_=ReloadQueue{}; mobileAim_=MobileAim{};
            feedback_=GameFeedbackSystem{}; mobileNotice_.clear();
            OnEnter(context); return false;
        default: break;
    }
    if(controls.CurrentMenu()!=Menu::Gameplay) {
        player_.SetBracing(false);
        if(controls.CurrentMenu()!=Menu::Weapons) return false;
    }
    if(context.input && context.input->AimChanged() && controls.HasAim())
        mobileAim_.Update(player_.Position(),camera_.ScreenToWorld(context.input->MousePosition()));
    mobileAutoSaveSeconds_+=std::min(std::max(deltaSeconds,0.0f),0.1f);
    if(mobileAutoSaveSeconds_>=3.0f) { SaveMobileCheckpoint(); mobileAutoSaveSeconds_=0; }
    return true;
}
void GameplayScene::DrawMobile() const {
    using namespace mobile;
    const auto& controls=Controls();
    Button(InfoButton,"HELP"); Button(BraceButton,"BRACE",player_.IsBracing()); Button(WeaponsButton,"GUNS");
    const Gun* gun=player_.Weapons().Current();
    const char* status=gun && gun->IsReloading()?"RELOADING (ground only)":
        mobileReload_.Pending(player_.Weapons().CurrentSlot())?"RELOAD QUEUED - land to reload":
        gun && gun->AmmoInMagazine()==0?"EMPTY - tap once to queue reload":"TAP TO SHOOT | HOLD SMG | BACK: PAUSE";
    DrawText(status,168,678,19,Color{183,214,239,255});
    DrawText(TextFormat("%d FPS  |  v0.1.0",GetFPS()),1010,20,18,Color{145,179,207,255});
    if(controls.CurrentMenu()==Menu::Gameplay) return;
    DrawRectangle(0,0,1280,720,Color{6,12,21,190});
    DrawRectangleRounded({190,112,900,514},0.06f,12,Color{13,25,41,245});
    Button(CloseButton,"BACK");
    if(controls.CurrentMenu()==Menu::Weapons) {
        DrawText("CHOOSE YOUR RECOIL",250,158,32,RAYWHITE);
        DrawText("World time: 8% | Each gun keeps its own magazine",250,205,20,Color{155,194,226,255});
        for(int slot=0;slot<3;++slot) {
            const Gun* weapon=player_.Weapons().At(slot); if(!weapon) continue;
            Rectangle r=WeaponButton(slot); Button(r,"",slot==player_.Weapons().CurrentSlot());
            const auto& d=weapon->Definition();
            DrawText(d.displayName.c_str(),static_cast<int>(r.x+18),static_cast<int>(r.y+28),22,RAYWHITE);
            DrawText(TextFormat("%d / %d",weapon->AmmoInMagazine(),d.magazineSize),static_cast<int>(r.x+18),static_cast<int>(r.y+78),34,Color{120,205,255,255});
            DrawText(TextFormat("Recoil %.0f",d.recoilForce),static_cast<int>(r.x+18),static_cast<int>(r.y+142),20,Color{175,207,233,255});
        }
        Button(ReloadButton,"RELOAD SELECTED GUN");
    } else if(controls.CurrentMenu()==Menu::Pause) {
        DrawText("PAUSED",380,155,36,RAYWHITE);
        Button(ResumeButton,"RESUME"); Button(SaveButton,"SAVE SAFE CHECKPOINT");
        Button(RestartButton,"RESTART TEST FIELD");
        Button(AssistButton,mobileAssist_?"BULLET AIM ASSIST: ON":"BULLET AIM ASSIST: OFF");
        Button(ShakeButton,mobileShake_?"CAMERA SHAKE: ON":"CAMERA SHAKE: OFF");
        DrawText(mobileNotice_.c_str(),380,590,18,Color{145,214,247,255});
    } else {
        DrawText("RECOIL JUMP MAN",250,158,34,RAYWHITE);
        const char* lines[]={
            "Tap the world: shoot there, recoil in the opposite direction.",
            "Revolver / shotgun: tap. SMG: hold, drag to redirect.",
            "Empty magazine: release and tap once to queue a reload.",
            "Reload starts on landing. Release before firing again.",
            "Hold BRACE on the ground to reduce recoil.",
            "GUNS opens slow motion. Android Back opens pause.",
            "Returning from another app stays paused until you resume.",
            "Checkpoint restores safe position; enemies/ammo reset.",
            "Prototype: no joystick, jump button, gyro or story yet."};
        for(int i=0;i<9;++i) DrawText(lines[i],250,226+i*39,20,Color{194,217,237,255});
    }
}
}
