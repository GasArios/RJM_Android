#!/usr/bin/env bash
set -euo pipefail
raylib_include=${1:?Pass the directory containing raylib.h}
mkdir -p build/tests
g++ -std=c++17 -O2 -Wall -Wextra -ffunction-sections -fdata-sections \
  -Igame/include -I"$raylib_include" tests/core_tests.cpp \
  game/src/Mobile/TouchControls.cpp game/src/Core/ViewportScaler.cpp \
  game/src/Weapons/Gun.cpp game/src/Weapons/WeaponInventory.cpp \
  game/src/Camera/GameCamera.cpp game/src/Physics/CoordinateSpace.cpp \
  game/src/Combat/AimAssistResolver.cpp game/src/Data/WeaponStatCalculator.cpp game/src/Physics/RecoilMovementController.cpp \
  game/src/Physics/TileCollisionResolver.cpp game/src/World/TileMap.cpp \
  -Wl,--gc-sections -o build/tests/core-tests
build/tests/core-tests
