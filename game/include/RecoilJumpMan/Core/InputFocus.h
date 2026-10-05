#pragma once

// InputFocus.h
// - 현재 입력이 어느 화면/패널에 집중되어 있는지 표현합니다.
// - 궤적 미리보기, 조준, 발사 같은 플레이 입력은 Gameplay focus에서만 켜는 것이 기본입니다.

namespace rjm
{
    // InputFocus:
    // - 현재 마우스/키보드 입력을 어떤 화면이 받아야 하는지 나타냅니다.
    // - 예를 들어 인벤토리가 열려 있을 때 좌클릭은 "총 발사"가 아니라
    //   "아이템 선택"으로 해석되어야 합니다.
    // - 이런 입력 소유권을 한 값으로 표현하면, GameplayScene 곳곳에서
    //   "인벤토리 열렸나? 지도 열렸나? 대화 중인가?"를 따로 검사하지 않아도 됩니다.
    enum class InputFocus
    {
        // Gameplay:
        // - 일반 플레이 상태입니다.
        // - 조준, 발사, 재장전, 무기 교체, 궤적 미리보기 같은 플레이 입력을 허용합니다.
        Gameplay,

        // Inventory:
        // - 가방/장비/아이템 UI가 입력을 받는 상태입니다.
        // - 이 상태에서는 마우스를 움직여도 총 조준 보조 UI가 계속 따라다니지 않는 것이 자연스럽습니다.
        Inventory,

        // Map:
        // - 지도 화면이 입력을 받는 상태입니다.
        // - 지도 드래그, 확대/축소, 마커 선택 같은 입력이 우선합니다.
        Map,

        // Dialogue:
        // - NPC 대화나 컷신 대사가 입력을 받는 상태입니다.
        // - 대화 중 실수로 발사하거나 재장전하지 않도록 플레이 입력을 막습니다.
        Dialogue,

        // Shop:
        // - 상점 UI가 입력을 받는 상태입니다.
        // - 구매/판매 선택이 우선하므로 플레이 조작은 꺼둡니다.
        Shop,

        // Settings:
        // - 설정창이 입력을 받는 상태입니다.
        // - 마우스 위치가 슬라이더나 버튼 위에 있어도 궤적 미리보기가 보이지 않아야 합니다.
        Settings,

        // Pause:
        // - 일시정지 메뉴가 입력을 받는 상태입니다.
        // - 게임플레이 시간이 멈춰 있거나 메뉴 조작이 우선인 상황입니다.
        Pause
    };

    // AllowsGameplayControls:
    // - 현재 focus에서 플레이어 조작을 받아도 되는지 판단합니다.
    // - inline 함수:
    //   헤더에 구현이 들어가지만 C++17 inline 덕분에 여러 .cpp에서 include해도 중복 정의 오류가 나지 않습니다.
    // - 지금은 Gameplay에서만 true지만, 나중에 "사진 모드에서 이동만 허용" 같은 예외가 생기면 이곳에서 조정할 수 있습니다.
    inline bool AllowsGameplayControls(InputFocus focus)
    {
        return focus == InputFocus::Gameplay;
    }

    // AllowsTrajectoryPreview:
    // - 궤적 미리보기를 보여도 되는지 판단합니다.
    // - 현재는 플레이 조작 허용 여부와 같은 기준을 쓰지만, 별도 함수로 둔 이유가 있습니다.
    // - 나중에 "발사는 막지만 조준 미리보기만 보여주는 튜토리얼" 같은 상태가 생기면
    //   AllowsGameplayControls와 다른 규칙을 줄 수 있습니다.
    inline bool AllowsTrajectoryPreview(InputFocus focus)
    {
        return AllowsGameplayControls(focus);
    }
}

