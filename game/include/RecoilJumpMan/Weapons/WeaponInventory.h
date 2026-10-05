#pragma once

// WeaponInventory.h
// - 플레이어가 장착한 총 3개를 관리하는 클래스입니다.
// - 각 총은 자기 탄창 상태와 재장전 상태를 따로 유지합니다.

#include "RecoilJumpMan/Weapons/Gun.h"

#include <array>
#include <optional>

namespace rjm
{
    // ReloadBatchResult:
    // - 한 번의 재장전 입력으로 장착 총 세트 재장전이 시작되었을 때의 결과입니다.
    // - 지금은 사운드 시스템이 없어서 사용하지 않지만, 나중에 "재장전 시작 소리 1번"을 재생할 때
    //   startedCount나 reloadSeconds 같은 정보를 참고할 수 있습니다.
    struct ReloadBatchResult
    {
        // 재장전 입력이 현재 상황에서 유효한 요청으로 받아들여졌는지 나타냅니다.
        // 예: EmptyMagazineOnly인데 현재 무기에 탄이 남아 있으면 false입니다.
        bool accepted = false;

        // 실제로 이번 입력으로 재장전을 새로 시작한 무기 수입니다.
        int startedCount = 0;

        // 입력 당시 선택되어 있던 무기 슬롯입니다.
        int currentSlot = -1;

        // 선택 중인 무기가 이번 입력으로 재장전을 새로 시작했는지 나타냅니다.
        bool currentSlotStarted = false;

        // 이번 입력으로 시작한 세트 재장전 시간입니다.
        // 이름은 기존 호출부 호환을 위해 유지합니다.
        float longestReloadSeconds = 0.0f;

        // StartedAny:
        // - 하나 이상의 무기가 실제로 재장전을 시작했는지 확인합니다.
        bool StartedAny() const;
    };

    class WeaponInventory
    {
    public:
        // static constexpr:
        // - 클래스에 속한 컴파일 타임 상수입니다.
        // - 플레이어가 장착할 수 있는 총 슬롯 수입니다.
        static constexpr int SlotCount = 3;

        // Equip:
        // - 특정 슬롯에 총을 장착합니다.
        // - 성공하면 true, 잘못된 슬롯이면 false를 반환합니다.
        bool Equip(int slot, Gun gun);

        // Select:
        // - 현재 사용할 총 슬롯을 선택합니다.
        bool Select(int slot);

        // SelectNextReadyWeapon:
        // - 현재 슬롯 다음부터 순서대로 보면서 즉시 발사 가능한 총을 선택합니다.
        // - 탄이 있고, 쿨타임이 끝났고, 재장전 중이 아닌 총만 선택합니다.
        // - 성공하면 currentSlot_이 바뀌고 true를 반환합니다.
        bool SelectNextReadyWeapon();

        // Update:
        // - 모든 장착 총의 쿨타임/재장전 상태를 갱신합니다.
        void Update(float deltaSeconds, bool grounded);

        // TryReloadAll:
        // - 한 번의 재장전 입력으로 장착 총 세트 재장전을 시작합니다.
        // - 재장전 시간은 장착된 총 전체 reloadSeconds 평균을 0.20~2.00초로 제한한 값입니다.
        // - EmptyMagazineOnly는 현재 선택 무기가 0발일 때만 세트 재장전을 시작합니다.
        ReloadBatchResult TryReloadAll(bool grounded, ReloadIntent intent);

        // Current:
        // - 현재 선택한 총을 수정 가능한 포인터로 반환합니다.
        // - 총이 없으면 nullptr를 반환합니다.
        Gun* Current();

        // const 버전 Current:
        // - 읽기 전용 상황에서 현재 총을 확인할 수 있습니다.
        const Gun* Current() const;
        Gun* At(int slot);
        const Gun* At(int slot) const;

        // CurrentSlot:
        // - 현재 선택된 슬롯 번호를 반환합니다.
        int CurrentSlot() const;

        // TotalWeight:
        // - 장착된 총들의 무게 합계를 반환합니다.
        int TotalWeight() const;

        // HasAnyLoadedWeapon:
        // - 장착 총 중 탄창에 탄이 1발 이상 남아 있는 총이 있는지 확인합니다.
        // - "정말 모든 총이 비었는가"를 판단할 때 사용합니다.
        bool HasAnyLoadedWeapon() const;

    private:
        // IsAnyReloading:
        // - 장착 총 중 하나라도 세트 재장전 상태라면 true입니다.
        bool IsAnyReloading() const;

        // CountReloadTargets:
        // - 현재 탄창이 가득 차 있지 않은 장착 총 개수를 셉니다.
        int CountReloadTargets() const;

        // CalculateSetReloadSeconds:
        // - 장착 총 전체 reloadSeconds 평균을 기반으로 세트 재장전 시간을 계산합니다.
        float CalculateSetReloadSeconds() const;

        // StartSetReload:
        // - 장착 총 전체를 같은 재장전 타이머로 묶습니다.
        void StartSetReload(float reloadSeconds);

        // std::array:
        // - 크기가 컴파일 시점에 정해진 배열입니다.
        //
        // std::optional<Gun>:
        // - 값이 있을 수도 있고 없을 수도 있음을 표현합니다.
        // - 총 슬롯이 비어 있을 수 있으므로 optional을 사용합니다.
        std::array<std::optional<Gun>, SlotCount> slots_;

        // 현재 선택된 무기 슬롯입니다.
        int currentSlot_ = 0;
    };
}
