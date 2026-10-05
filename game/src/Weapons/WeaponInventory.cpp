// WeaponInventory.cpp
// - WeaponInventory 클래스의 구현부입니다.
// - 총 장착, 선택, 상태 갱신, 무게 계산을 담당합니다.

#include "RecoilJumpMan/Weapons/WeaponInventory.h"

#include <algorithm>
#include <utility>

namespace rjm
{
    Gun* WeaponInventory::At(int slot) {
        return slot >= 0 && slot < SlotCount && slots_[slot] ? &*slots_[slot] : nullptr;
    }
    const Gun* WeaponInventory::At(int slot) const {
        return slot >= 0 && slot < SlotCount && slots_[slot] ? &*slots_[slot] : nullptr;
    }
    namespace
    {
        constexpr float MinimumSetReloadSeconds = 0.20f;
        constexpr float MaximumSetReloadSeconds = 2.00f;
    }

    bool ReloadBatchResult::StartedAny() const
    {
        return startedCount > 0;
    }

    // Equip:
    // - slot 번호가 유효하면 해당 슬롯에 총을 넣습니다.
    bool WeaponInventory::Equip(int slot, Gun gun)
    {
        // 범위를 벗어난 슬롯이면 실패입니다.
        if (slot < 0 || slot >= SlotCount)
        {
            return false;
        }

        // Gun 객체를 슬롯으로 이동합니다.
        slots_[slot] = std::move(gun);
        return true;
    }

    // Select:
    // - slot 번호가 유효하고, 해당 슬롯에 총이 있을 때만 선택합니다.
    bool WeaponInventory::Select(int slot)
    {
        if (slot < 0 || slot >= SlotCount || !slots_[slot].has_value())
        {
            return false;
        }

        currentSlot_ = slot;
        return true;
    }

    bool WeaponInventory::SelectNextReadyWeapon()
    {
        // 현재 슬롯의 다음 슬롯부터 순서대로 봅니다.
        // 예: 현재 0번이면 1번, 2번 순서로 검사합니다.
        // currentSlot_ 자신은 제외합니다. 이 함수는 "다음 총으로 자동 교체"하기 위한 함수이기 때문입니다.
        for (int offset = 1; offset < SlotCount; ++offset)
        {
            const int slot = (currentSlot_ + offset) % SlotCount;
            if (!slots_[slot].has_value())
            {
                continue;
            }

            // CanFire는 탄약, 쿨타임, 재장전 상태를 모두 검사합니다.
            // 자동 교체 직후 바로 쏠 수 없는 총으로 바뀌면 플레이어가 더 헷갈릴 수 있어 즉시 발사 가능 총만 고릅니다.
            if (!slots_[slot]->CanFire())
            {
                continue;
            }

            currentSlot_ = slot;
            return true;
        }

        return false;
    }

    // Update:
    // - 모든 슬롯을 순회하면서 총이 있는 슬롯만 업데이트합니다.
    void WeaponInventory::Update(float deltaSeconds, bool grounded)
    {
        for (auto& slot : slots_)
        {
            // optional이 실제 값을 가지고 있는지 확인합니다.
            if (slot.has_value())
            {
                // optional 안의 Gun에 접근할 때 -> 연산자를 사용할 수 있습니다.
                slot->Update(deltaSeconds, grounded);
            }
        }
    }

    ReloadBatchResult WeaponInventory::TryReloadAll(bool grounded, ReloadIntent intent)
    {
        ReloadBatchResult result;
        result.currentSlot = currentSlot_;

        const Gun* currentGun = Current();
        if (intent == ReloadIntent::EmptyMagazineOnly)
        {
            // 빈 탄창 좌클릭 재장전은 "현재 들고 있는 총"이 비었을 때만 세트 재장전으로 해석합니다.
            // 현재 총이 없거나 탄이 남아 있으면 좌클릭은 재장전 요청으로 받아들이지 않습니다.
            if (!currentGun || currentGun->AmmoInMagazine() > 0)
            {
                return result;
            }
        }

        const int reloadTargetCount = CountReloadTargets();
        if (reloadTargetCount <= 0)
        {
            return result;
        }

        result.accepted = true;

        // 이미 세트 재장전 중이면 입력은 유효하지만 새 타이머로 덮어쓰지 않습니다.
        if (!grounded || IsAnyReloading())
        {
            return result;
        }

        const float setReloadSeconds = CalculateSetReloadSeconds();
        const Gun* selectedGun = Current();
        result.startedCount = reloadTargetCount;
        result.currentSlotStarted = selectedGun && selectedGun->NeedsReload();
        result.longestReloadSeconds = setReloadSeconds;
        StartSetReload(setReloadSeconds);

        return result;
    }

    // Current:
    // - 현재 슬롯에 총이 없으면 nullptr를 반환합니다.
    Gun* WeaponInventory::Current()
    {
        if (!slots_[currentSlot_].has_value())
        {
            return nullptr;
        }

        // optional 안의 실제 Gun 객체 주소를 반환합니다.
        return &slots_[currentSlot_].value();
    }

    // const 버전 Current:
    // - 위 함수와 같지만 읽기 전용 포인터를 반환합니다.
    const Gun* WeaponInventory::Current() const
    {
        if (!slots_[currentSlot_].has_value())
        {
            return nullptr;
        }

        return &slots_[currentSlot_].value();
    }

    // CurrentSlot:
    // - 현재 선택된 슬롯 번호를 반환합니다.
    int WeaponInventory::CurrentSlot() const
    {
        return currentSlot_;
    }

    // TotalWeight:
    // - 장착된 모든 총의 weight를 더합니다.
    int WeaponInventory::TotalWeight() const
    {
        int total = 0;

        for (const auto& slot : slots_)
        {
            if (slot.has_value())
            {
                total += slot->Definition().weight;
            }
        }

        return total;
    }

    bool WeaponInventory::HasAnyLoadedWeapon() const
    {
        for (const auto& slot : slots_)
        {
            if (slot.has_value() && slot->AmmoInMagazine() > 0)
            {
                return true;
            }
        }

        return false;
    }

    bool WeaponInventory::IsAnyReloading() const
    {
        for (const auto& slot : slots_)
        {
            if (slot.has_value() && slot->IsReloading())
            {
                return true;
            }
        }

        return false;
    }

    int WeaponInventory::CountReloadTargets() const
    {
        int count = 0;
        for (const auto& slot : slots_)
        {
            if (slot.has_value() && slot->NeedsReload())
            {
                ++count;
            }
        }

        return count;
    }

    float WeaponInventory::CalculateSetReloadSeconds() const
    {
        float totalReloadSeconds = 0.0f;
        int equippedCount = 0;

        for (const auto& slot : slots_)
        {
            if (!slot.has_value())
            {
                continue;
            }

            totalReloadSeconds += slot->Definition().reloadSeconds;
            ++equippedCount;
        }

        if (equippedCount <= 0)
        {
            return MinimumSetReloadSeconds;
        }

        const float averageReloadSeconds = totalReloadSeconds / static_cast<float>(equippedCount);
        return std::clamp(averageReloadSeconds, MinimumSetReloadSeconds, MaximumSetReloadSeconds);
    }

    void WeaponInventory::StartSetReload(float reloadSeconds)
    {
        for (auto& slot : slots_)
        {
            if (slot.has_value())
            {
                slot->StartReload(reloadSeconds);
            }
        }
    }
}
