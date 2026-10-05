#pragma once
namespace rjm::mobile {
// Screen distances use a 720-unit reference height, independent of camera zoom.
struct MobileTuning {
    static constexpr float DefaultZoom = 1.10f;
    static constexpr float AimDeadZone = 12.0f;
    static constexpr float DragThreshold = 10.0f;
    static constexpr float MinimumPinchSpan = 24.0f;
    static constexpr float PadRadius = 100.0f;
    static constexpr float PadKnobRadius = 28.0f;
    static constexpr float PadDeadZone = 18.0f;
    static constexpr float PadRightMargin = 50.0f;
    static constexpr float PadBottomMargin = 56.0f;
    static constexpr float SlowTimeScale = 0.08f;
};
enum class AimMode { Character = 0, ScreenCenter = 1, Joystick = 2, TouchCircle = 3 };
constexpr int AimModeCount = 4;
inline bool IsPadMode(AimMode mode) {return mode==AimMode::Joystick || mode==AimMode::TouchCircle;}
inline AimMode NextAimMode(AimMode mode) {return static_cast<AimMode>((static_cast<int>(mode)+1)%AimModeCount);}
inline const char* AimModeName(AimMode mode) {
    switch(mode) {
        case AimMode::Character:return "CHARACTER";
        case AimMode::ScreenCenter:return "SCREEN CENTER";
        case AimMode::Joystick:return "AIM PAD";
        case AimMode::TouchCircle:return "TOUCH CIRCLE";
    }
    return "CHARACTER";
}
}
