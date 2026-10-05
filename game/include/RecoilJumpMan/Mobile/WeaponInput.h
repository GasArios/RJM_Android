#pragma once
#include "RecoilJumpMan/Weapons/WeaponInventory.h"
namespace rjm::mobile {
struct WeaponFireRequest {Gun* gun=nullptr;bool fire=false,switched=false,reload=false;};
inline bool WantsFire(const Gun& gun,bool pressed,bool held,bool continuous=false) {
    return continuous || gun.FireMode()==FireControlMode::FullAuto ? held||pressed : pressed;
}
// Same ready-slot order and post-switch firing-mode rule as the PC version.
// Screen modes preserve semi-auto taps. Pad modes opt into continuous input
// for every gun and keep it through automatic switching; cooldowns stay in Gun.
inline WeaponFireRequest ResolveWeaponFire(WeaponInventory& weapons,bool pressed,bool held,bool continuous=false) {
    WeaponFireRequest result;result.gun=weapons.Current();
    if(!result.gun || !WantsFire(*result.gun,pressed,held,continuous)) return result;
    if(result.gun->AmmoInMagazine()<=0) {
        if(weapons.SelectNextReadyWeapon()) {
            result.gun=weapons.Current();result.switched=true;
        } else {
            result.reload=(pressed || (continuous && held)) && !weapons.HasAnyLoadedWeapon();return result;
        }
    }
    result.fire=WantsFire(*result.gun,pressed,held,continuous);
    return result;
}
}
