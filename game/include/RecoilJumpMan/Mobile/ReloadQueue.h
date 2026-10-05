#pragma once
#include "RecoilJumpMan/Weapons/WeaponInventory.h"
namespace rjm::mobile {
// Set-wide, non-expiring reservation. Original inventory calculates the shared
// timer from ALL equipped reloadSeconds, clamped to 0.20 .. 2.00 seconds.
class ReloadQueue {
public:
    void Request() {queued_=true;}
    bool Pending() const {return queued_;}
    void Update(WeaponInventory& weapons,bool grounded) {
        if(!queued_) return;
        if(weapons.CountReloadTargets()==0 || weapons.IsAnyReloading()) {queued_=false;return;}
        if(grounded && weapons.TryReloadAll(true,ReloadIntent::Manual).StartedAny()) queued_=false;
    }
private:
    bool queued_=false;
};
}
