#pragma once
#include "RecoilJumpMan/Weapons/WeaponInventory.h"
namespace rjm::mobile {
struct WeaponFireRequest {Gun* gun=nullptr;bool fire=false,switched=false,reload=false;};
inline bool WantsFire(const Gun& gun,bool pressed,bool held) {
    return gun.FireMode()==FireControlMode::FullAuto ? held||pressed : pressed;
}
// Same ready-slot order and post-switch firing-mode rule as the PC version.
// A held SMG cannot fire the newly selected semi-auto without a fresh press.
inline WeaponFireRequest ResolveWeaponFire(WeaponInventory& weapons,bool pressed,bool held) {
    WeaponFireRequest result;result.gun=weapons.Current();
    if(!result.gun || !WantsFire(*result.gun,pressed,held)) return result;
    if(result.gun->AmmoInMagazine()<=0) {
        if(weapons.SelectNextReadyWeapon()) {
            result.gun=weapons.Current();result.switched=true;
        } else {
            result.reload=pressed && !weapons.HasAnyLoadedWeapon();return result;
        }
    }
    result.fire=WantsFire(*result.gun,pressed,held);
    return result;
}
}
