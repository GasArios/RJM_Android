#pragma once
#include "RecoilJumpMan/Weapons/WeaponInventory.h"
#include <array>
namespace rjm::mobile {
class ReloadQueue {
public:
    void Request(int slot) { if(slot>=0 && slot<3) queued_[slot]=true; }
    bool Pending(int slot) const { return slot>=0 && slot<3 && queued_[slot]; }
    void Update(WeaponInventory& weapons, bool grounded) {
        if(!grounded) return;
        for(int slot=0;slot<3;++slot) if(queued_[slot]) {
            Gun* gun=weapons.At(slot);
            if(!gun || !gun->NeedsReload() || gun->IsReloading() || gun->TryReload(true,ReloadIntent::Manual)) queued_[slot]=false;
        }
    }
private:
    std::array<bool,3> queued_{};
};
}
