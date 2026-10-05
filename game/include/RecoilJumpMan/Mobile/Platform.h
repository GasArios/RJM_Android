#pragma once
#include "RecoilJumpMan/Mobile/TouchControls.h"
#include <string>
#include <vector>
namespace rjm::mobile {
std::vector<TouchEvent> DrainTouchEvents();
bool ConsumeBack();
bool ConsumePause();
std::string StoragePath();
}
