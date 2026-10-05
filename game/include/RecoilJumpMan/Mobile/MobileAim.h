#pragma once
#include <raylib.h>
#include <cmath>
namespace rjm::mobile {
// Store a world direction, not a fixed screen/world point. Camera movement and
// recoil therefore cannot redirect an unchanged held touch.
class MobileAim {
public:
    void Update(Vector2 player, Vector2 target) {
        const float dx=target.x-player.x, dy=target.y-player.y;
        const float length=std::hypot(dx,dy);
        valid_=length>=8.0f;
        if(valid_) direction_={dx/length,dy/length};
    }
    bool Valid() const { return valid_; }
    Vector2 Target(Vector2 player) const { return {player.x+direction_.x*1000.0f,player.y+direction_.y*1000.0f}; }
private:
    Vector2 direction_{0,-1};
    bool valid_=false;
};
}
