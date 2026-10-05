#pragma once
#include <raylib.h>
#include <cmath>
#include "RecoilJumpMan/Mobile/AimPad.h"
namespace rjm::mobile {
// Sample the screen direction only on DOWN / intentional drag. Screen Y points
// down; world Y points up. Camera motion, zoom and recoil cannot redirect a hold.
class MobileAim {
public:
    void UpdateScreen(Vector2 playerScreen,Vector2 touch,Vector2 screenSize,AimMode mode) {
        const Vector2 origin=IsPadMode(mode)?AimPad::Center(screenSize):
            mode==AimMode::ScreenCenter?Vector2{screenSize.x*0.5f,screenSize.y*0.5f}:playerScreen;
        const float dx=touch.x-origin.x,dy=origin.y-touch.y;
        const float length=std::hypot(dx,dy);
        valid_=length>=(IsPadMode(mode)?MobileTuning::PadDeadZone:MobileTuning::AimDeadZone);
        if(valid_) direction_={dx/length,dy/length};
    }
    bool Valid() const { return valid_; }
    Vector2 Target(Vector2 player) const { return {player.x+direction_.x*1000,player.y+direction_.y*1000}; }
    Vector2 Direction() const {return direction_;}
private:
    Vector2 direction_{0,-1};
    bool valid_=false;
};
}
