#pragma once

// HitEffectSystem.h
// - HitEvent를 받아 피격 시각 효과를 만들어내는 시스템입니다.
// - 전투 코드는 "누가 어디서 누구를 맞혔다"는 사실만 만들고,
//   이 시스템은 그 사실을 화면에서 어떻게 보여줄지만 담당합니다.

#include "RecoilJumpMan/Combat/HitEvent.h"

#include <raylib.h>

#include <cstddef>
#include <vector>

namespace rjm
{
    struct HitEffectParticle
    {
        // 파티클의 월드 좌표입니다.
        // 카메라 안에서 그려지므로 플레이어/적과 같은 좌표계를 씁니다.
        Vector2 position = { 0.0f, 0.0f };

        // 파티클의 월드 속도입니다.
        // y-up 물리 좌표계이므로 y가 양수면 위로 움직입니다.
        Vector2 velocity = { 0.0f, 0.0f };

        // 현재 남은 수명입니다.
        float lifetimeSeconds = 0.0f;

        // 처음 생성되었을 때의 수명입니다.
        // Draw에서 남은 수명 비율로 alpha와 크기를 줄이는 데 사용합니다.
        float maxLifetimeSeconds = 0.0f;

        // 그릴 원의 기본 반지름입니다.
        float radius = 2.0f;

        // 파티클 색상입니다.
        // DamageType에 따라 기본 색을 다르게 줄 수 있습니다.
        Color color = WHITE;
    };

    class HitEffectSystem
    {
    public:
        // EmitHit:
        // - 명중 이벤트 하나를 받아 피격 파티클 여러 개를 생성합니다.
        // - 나중에 hitEffectId를 보고 전용 이펙트 프리셋을 고르는 입구가 될 수 있습니다.
        void EmitHit(const HitEvent& event);

        // Update:
        // - 파티클 위치와 남은 수명을 갱신합니다.
        void Update(float deltaSeconds);

        // Draw:
        // - 모든 파티클을 월드 좌표 기준으로 그립니다.
        // - GameplayScene에서 camera_.Begin()과 camera_.End() 사이에 호출됩니다.
        void Draw() const;

        // Clear:
        // - 씬 재진입, 레벨 전환, 리셋 시 남아 있던 파티클을 모두 지웁니다.
        void Clear();

    private:
        // SpawnParticle:
        // - 파티클 목록에 새 파티클을 추가합니다.
        // - 상한을 넘으면 가장 오래된 파티클을 제거해 메모리와 렌더 비용을 제한합니다.
        void SpawnParticle(const HitEffectParticle& particle);

        // ColorForDamage:
        // - 피해 속성에 따라 기본 피격 색을 고릅니다.
        static Color ColorForDamage(const Damage& damage);

        // 현재 살아있는 피격 파티클 목록입니다.
        std::vector<HitEffectParticle> particles_;

        // 한 번에 유지할 최대 파티클 수입니다.
        // 연사 무기나 폭발탄이 많아져도 화면과 성능이 과하게 흔들리지 않게 합니다.
        std::size_t maxParticles_ = 160;
    };
}

