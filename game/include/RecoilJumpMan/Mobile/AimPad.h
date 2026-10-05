#pragma once
#include "RecoilJumpMan/Mobile/MobileTuning.h"
#include <raylib.h>
#include <cmath>
#include <algorithm>
namespace rjm::mobile {
// Shared input/render geometry. Distance changes neither recoil nor fire rate.
struct AimPad {
    static Vector2 Center(Vector2 size) {return {size.x-MobileTuning::PadRadius-MobileTuning::PadRightMargin,
                                                size.y-MobileTuning::PadRadius-MobileTuning::PadBottomMargin};}
    static float Distance(Vector2 size,Vector2 point) {
        const auto c=Center(size);return std::hypot(point.x-c.x,point.y-c.y);
    }
    static bool Contains(Vector2 size,Vector2 point) {return Distance(size,point)<=MobileTuning::PadRadius;}
    static bool Active(Vector2 size,Vector2 point) {return Distance(size,point)>=MobileTuning::PadDeadZone;}
    static Vector2 Knob(Vector2 size,Vector2 point,bool touched) {
        const auto c=Center(size);if(!touched) return c;
        const float dx=point.x-c.x,dy=point.y-c.y,distance=std::hypot(dx,dy);
        if(distance<=0) return c;
        const float scale=std::min(1.0f,(MobileTuning::PadRadius-MobileTuning::PadKnobRadius)/distance);
        return {c.x+dx*scale,c.y+dy*scale};
    }
};
}
