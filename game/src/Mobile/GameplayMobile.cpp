#include "RecoilJumpMan/Scene/GameplayScene.h"
#include "RecoilJumpMan/Core/GameContext.h"
#include "RecoilJumpMan/Core/InputState.h"
#include "RecoilJumpMan/Core/ViewportScaler.h"
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
// Keep the filename so existing 0.1.0 checkpoints can be read and migrated.
std::string CheckpointPath() {return rjm::mobile::StoragePath()+"/checkpoint-v1.txt";}
}
namespace rjm {
bool GameplayScene::SaveMobileCheckpoint() const {
    const Vector2 position=ResolveRecoveryPosition();
    const std::string path=CheckpointPath(),temp=path+".tmp";
    std::ofstream file(temp,std::ios::trunc);
    if(!file) return false;
    file<<"RJM_CHECKPOINT_2\n"<<position.x<<' '<<position.y<<' '
        <<player_.Weapons().CurrentSlot()<<' '<<mobileAssist_<<' '<<mobileShake_<<' '
        <<static_cast<int>(mobileAimMode_)<<' '<<camera_.Zoom()<<'\n';
    file.flush();if(!file) return false;
    file.close();return std::rename(temp.c_str(),path.c_str())==0;
}
void GameplayScene::LoadMobileCheckpoint() {
    std::ifstream file(CheckpointPath());
    std::string version;Vector2 position{};int slot=0,assist=1,shake=1,mode=0;
    float zoom=mobile::MobileTuning::DefaultZoom;
    if(!(file>>version>>position.x>>position.y>>slot>>assist>>shake) ||
       (version!="RJM_CHECKPOINT_1" && version!="RJM_CHECKPOINT_2")) return;
    if(version=="RJM_CHECKPOINT_2" && (!(file>>mode>>zoom) || mode<0 || mode>=mobile::AimModeCount || !std::isfinite(zoom))) return;
    const Rectangle bounds=level_.WorldBounds();const float half=player_.HalfBodySize();
    if(!std::isfinite(position.x)||!std::isfinite(position.y)||slot<0||slot>=3||
       position.x<bounds.x+half||position.x>bounds.x+bounds.width-half||
       position.y<bounds.y+half||position.y>bounds.y+bounds.height-half) return;
    const auto& map=level_.World().Map();
    for(float x:{-half+1,half-1}) for(float y:{-half+1,half-1})
        if(map.IsSolidAtWorld({position.x+x,position.y+y})) return;
    player_.SetPosition(position);player_.SetVelocity({});
    player_.Weapons().Select(slot);lastSafePosition_=position;
    mobileAssist_=assist!=0;mobileShake_=shake!=0;mobileAimMode_=static_cast<mobile::AimMode>(mode);
    camera_.SetZoomAnchored(zoom,position);
    TraceLog(LOG_INFO,"RJM: restored aim=%d zoom=%.3f",mode,camera_.Zoom());
}
bool GameplayScene::UpdateMobile(GameContext& context,float deltaSeconds) {
    using namespace mobile;
    const auto& controls=Controls();
    const Vector2 size=controls.ScreenSize();
    camera_.SetViewportSize(size.x,size.y);
    if(controls.PinchScale()!=1.0f) camera_.SetZoomAnchored(camera_.Zoom()*controls.PinchScale(),player_.Position());
    // A short gesture can start and finish in one input batch while paused.
    if(!controls.Pinching() && (mobileWasPinching_ || controls.PinchScale()!=1.0f)) {
        SaveMobileCheckpoint();TraceLog(LOG_INFO,"RJM: zoom=%.3f",camera_.Zoom());
    }
    mobileWasPinching_=controls.Pinching();
    const int menu=static_cast<int>(controls.CurrentMenu());
    if(menu!=mobilePreviousMenu_) {
        TraceLog(LOG_INFO,"RJM: menu=%d",menu);
        if(controls.CurrentMenu()==Menu::Pause) SaveMobileCheckpoint();
        mobilePreviousMenu_=menu;
    }
    switch(controls.RequestedAction()) {
        case Action::Save:mobileNotice_=SaveMobileCheckpoint()?"Safe checkpoint saved":"Save failed";break;
        case Action::ToggleAssist:mobileAssist_=!mobileAssist_;SaveMobileCheckpoint();break;
        case Action::ToggleShake:mobileShake_=!mobileShake_;SaveMobileCheckpoint();break;
        case Action::ToggleAimMode:
            mobileAimMode_=NextAimMode(mobileAimMode_);
            Controls().SetAimMode(mobileAimMode_);
            mobileAim_=MobileAim{};SaveMobileCheckpoint();
            TraceLog(LOG_INFO,"RJM: aim mode=%d",static_cast<int>(mobileAimMode_));break;
        case Action::ResetZoom:
            camera_.SetZoomAnchored(MobileTuning::DefaultZoom,player_.Position());SaveMobileCheckpoint();break;
        case Action::Restart:
            std::remove(CheckpointPath().c_str());
            player_=Player{};mobileReload_=ReloadQueue{};mobileAim_=MobileAim{};
            feedback_=GameFeedbackSystem{};mobileNotice_.clear();
            OnEnter(context);return false;
        default:break;
    }
    if(controls.CurrentMenu()!=Menu::Gameplay) {
        player_.SetBracing(false);
        if(controls.CurrentMenu()!=Menu::Weapons) return false;
    }
    if(context.input && context.input->AimChanged() && controls.HasAim())
        mobileAim_.UpdateScreen(camera_.WorldToScreen(player_.Position()),context.input->MousePosition(),size,mobileAimMode_);
    mobileAutoSaveSeconds_+=std::min(std::max(deltaSeconds,0.0f),0.1f);
    if(mobileAutoSaveSeconds_>=3) {SaveMobileCheckpoint();mobileAutoSaveSeconds_=0;}
    return true;
}
void GameplayScene::DrawMobile() const {
    using namespace mobile;
    const auto& controls=Controls();const Vector2 size=controls.ScreenSize();
    const int shift=static_cast<int>((size.x-1280)*0.5f);
    Button(InfoButton,"HELP");Button(BraceButton,"BRACE",player_.IsBracing());
    Button(LeftReloadButton,"RELOAD");Button(WeaponsButton,"GUNS");
    const Gun* gun=player_.Weapons().Current();
    const char* status=player_.Weapons().IsAnyReloading()?"RELOADING ALL (ground only)":
        mobileReload_.Pending()?"SET RELOAD QUEUED - land to reload":
        gun && gun->AmmoInMagazine()==0?"EMPTY - next input switches gun":IsPadMode(mobileAimMode_)?"PAD: HOLD TO FIRE | CENTER / RELEASE: STOP":"TAP: SHOOT | TWO FINGERS: ZOOM | BACK: PAUSE";
    DrawText(status,168,678,19,Color{183,214,239,255});
    DrawText(TextFormat("%d FPS | 0.1.2 | %.2fx",GetFPS(),camera_.Zoom()),static_cast<int>(size.x)-310,20,18,Color{145,179,207,255});
    DrawText(TextFormat("%d/4: %s",static_cast<int>(mobileAimMode_)+1,AimModeName(mobileAimMode_)),static_cast<int>(size.x)-310,48,18,Color{145,179,207,255});
    if(controls.CurrentMenu()==Menu::Gameplay) {
        if(IsPadMode(mobileAimMode_)) {
            const auto center=AimPad::Center(size);
            const Color outline=controls.PadBlocked()?Color{245,177,89,210}:Color{144,198,233,180};
            DrawCircleV(center,MobileTuning::PadRadius,Color{25,52,76,95});
            DrawCircleLines(static_cast<int>(center.x),static_cast<int>(center.y),MobileTuning::PadRadius,outline);
            DrawCircleLines(static_cast<int>(center.x),static_cast<int>(center.y),MobileTuning::PadDeadZone,Color{160,200,230,90});
            if(controls.PadTouched()) {
                const auto knob=controls.PadKnob();
                DrawLineEx(center,knob,3,outline);
            }
            if(mobileAimMode_==AimMode::Joystick) {
                DrawCircleV(controls.PadKnob(),MobileTuning::PadKnobRadius,Color{87,164,215,195});
                DrawCircleLines(static_cast<int>(controls.PadKnob().x),static_cast<int>(controls.PadKnob().y),MobileTuning::PadKnobRadius,outline);
            }
            const char* label=controls.PadBlocked()?"RELEASE TO REARM":mobileAimMode_==AimMode::Joystick?"AIM PAD":"TOUCH CIRCLE";
            DrawText(label,static_cast<int>(center.x)-MeasureText(label,17)/2,static_cast<int>(center.y+MobileTuning::PadRadius+10),17,outline);
        }
        if(mobileAimMode_==AimMode::ScreenCenter) {
            const Vector2 center{size.x*0.5f,size.y*0.5f};
            DrawCircleLines(static_cast<int>(center.x),static_cast<int>(center.y),MobileTuning::AimDeadZone,Color{175,213,240,100});
            DrawLineEx({center.x-20,center.y},{center.x+20,center.y},1,Color{175,213,240,80});
            DrawLineEx({center.x,center.y-20},{center.x,center.y+20},1,Color{175,213,240,80});
        }
        if(mobileAim_.Valid()) {
            const Vector2 start=camera_.WorldToScreen(player_.Position()),dir=mobileAim_.Direction();
            DrawLineEx(start,{start.x+dir.x*38,start.y-dir.y*38},3,Color{111,215,255,180});
        }
        if(controls.Pinching()) DrawText(TextFormat("ZOOM %.2fx",camera_.Zoom()),shift+540,120,28,RAYWHITE);
        return;
    }
    DrawRectangle(0,0,static_cast<int>(size.x),static_cast<int>(size.y),Color{6,12,21,190});
    DrawRectangleRounded({190.0f+shift,112,900,565},0.06f,12,Color{13,25,41,245});
    Button(CloseButton(),"BACK");
    if(controls.CurrentMenu()==Menu::Weapons) {
        DrawText("CHOOSE YOUR RECOIL",shift+250,158,32,RAYWHITE);
        DrawText("World time: 8% | Empty guns switch on the next shot input",shift+250,205,20,Color{155,194,226,255});
        for(int slot=0;slot<3;++slot) {
            const Gun* weapon=player_.Weapons().At(slot);if(!weapon) continue;
            Rectangle r=WeaponButton(slot);Button(r,"",slot==player_.Weapons().CurrentSlot());
            const auto& d=weapon->Definition();
            DrawText(d.displayName.c_str(),static_cast<int>(r.x+18),static_cast<int>(r.y+28),22,RAYWHITE);
            DrawText(TextFormat("%d / %d",weapon->AmmoInMagazine(),d.magazineSize),static_cast<int>(r.x+18),static_cast<int>(r.y+78),34,Color{120,205,255,255});
            DrawText(TextFormat("Recoil %.0f",d.recoilForce),static_cast<int>(r.x+18),static_cast<int>(r.y+142),20,Color{175,207,233,255});
        }
        Button(ReloadButton(),"RELOAD ALL GUNS / QUEUE ON LANDING");
        DrawText(TextFormat("Shared reload: %.2fs (average of all equipped guns)",player_.Weapons().CalculateSetReloadSeconds()),shift+250,603,20,Color{155,194,226,255});
    } else if(controls.CurrentMenu()==Menu::Pause) {
        DrawText("PAUSED - pinch here to adjust zoom",shift+360,155,26,RAYWHITE);
        Button(ResumeButton(),"RESUME");Button(SaveButton(),"SAVE SAFE CHECKPOINT");
        Button(RestartButton(),"RESTART TEST FIELD");
        Button(AssistButton(),mobileAssist_?"BULLET AIM ASSIST: ON":"BULLET AIM ASSIST: OFF");
        Button(ShakeButton(),mobileShake_?"CAMERA SHAKE: ON":"CAMERA SHAKE: OFF");
        Button(AimModeButton(),TextFormat("CONTROL %d/4: %s (tap to cycle)",static_cast<int>(mobileAimMode_)+1,AimModeName(mobileAimMode_)));
        Button(ZoomResetButton(),TextFormat("ZOOM %.2fx - RESET TO 1.10x",camera_.Zoom()));
        DrawText(mobileNotice_.c_str(),shift+360,642,18,Color{145,214,247,255});
    } else {
        DrawText("RECOIL JUMP MAN",shift+250,158,34,RAYWHITE);
        const char* lines[]={
            "Tap to shoot. Recoil moves you in the opposite direction.",
            "CONTROL in pause cycles 1: character / 2: screen center.",
            "3: aim pad / 4: touch circle. Both use the same fixed center.",
            "Modes 3/4: all guns repeat while held outside the small center.",
            "Modes 1/2: SMG holds; revolver / shotgun use fresh taps.",
            "Empty gun: next shot input switches to the next ready gun.",
            "All empty: pad hold or fresh screen tap queues a set reload.",
            "Release after a reload request. Left RELOAD refills all guns.",
            "Pinch with two world fingers to zoom; pause allows safe zoom.",
            "Pad / left buttons never become pinch fingers or other buttons.",
            "Hold BRACE on ground. Back pauses; app return stays paused.",
            "Prototype checkpoint restores position/settings; ammo resets."};
        for(int i=0;i<12;++i) DrawText(lines[i],shift+250,224+i*34,18,Color{194,217,237,255});
    }
}
}
