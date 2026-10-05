// Hud.cpp
// - Hud 클래스의 구현부입니다.
// - 현재는 플레이어 HP와 현재 총 탄약을 텍스트로 표시합니다.

#include "RecoilJumpMan/UI/Hud.h"

#include "RecoilJumpMan/Data/WeaponStatCalculator.h"
#include "RecoilJumpMan/Player/Player.h"
#include "RecoilJumpMan/Weapons/Gun.h"

#include <raylib.h>

#include <cstdio>

namespace
{
    const char* FireModeText(rjm::FireControlMode mode)
    {
        return mode == rjm::FireControlMode::FullAuto ? "AUTO" : "SEMI";
    }
}

namespace rjm
{
    // Draw:
    // - Player를 const 참조로 받아 상태를 읽기만 합니다.
    void Hud::Draw(const Player& player) const
    {
        // char 배열:
        // - C 스타일 문자열을 담는 공간입니다.
        char hpText[64];

        // std::snprintf:
        // - 숫자를 문자열로 안전하게 포맷팅합니다.
        // - sizeof(hpText)를 넘지 않게 작성합니다.
        std::snprintf(hpText, sizeof(hpText), "HP %.0f / %.0f", player.HitPoints().Current(), player.HitPoints().Maximum());

        // DrawText는 Raylib의 텍스트 그리기 함수입니다.
        DrawText(hpText, 20, 20, 20, RAYWHITE);

        // 현재 선택된 총을 가져옵니다.
        const Gun* gun = player.Weapons().Current();
        if (gun)
        {
            char ammoText[128];
            std::snprintf(ammoText, sizeof(ammoText), "%s  [%s]  Ammo %d / %d",
                gun->Definition().displayName.c_str(),
                FireModeText(gun->FireMode()),
                gun->AmmoInMagazine(),
                gun->Definition().magazineSize);
            DrawText(ammoText, 20, 48, 20, Color{ 210, 225, 245, 255 });

            const WeaponDefinition& definition = gun->Definition();
            const float mobilityScore = CalculateWeaponMobilityScore(definition);
            char mobilityText[128];
            std::snprintf(mobilityText, sizeof(mobilityText), "Recoil %.0f  Alt %.1f  Mobility %.1f / 10",
                definition.recoilForce,
                definition.altitudeDampingResistance * 10.0f,
                mobilityScore);
            DrawText(mobilityText, 20, 74, 18, Color{ 170, 205, 230, 230 });
        }
    }
}

