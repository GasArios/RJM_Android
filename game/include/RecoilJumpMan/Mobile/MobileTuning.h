#pragma once
namespace rjm::mobile {
// Screen distances use a 720-unit reference height, independent of camera zoom.
struct MobileTuning {
    static constexpr float DefaultZoom = 1.10f;
    static constexpr float AimDeadZone = 12.0f;
    static constexpr float DragThreshold = 10.0f;
    static constexpr float MinimumPinchSpan = 24.0f;
    static constexpr float SlowTimeScale = 0.08f;
};
enum class AimMode { Character = 0, ScreenCenter = 1 };
}
