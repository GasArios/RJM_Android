// HitEffectSystem.cpp
// - HitEvent를 작은 월드 파티클로 표현합니다.
// - 지금은 Raylib primitive 원으로 그리지만, 나중에 스프라이트/파티클 에셋 기반으로 교체할 수 있습니다.

#include "RecoilJumpMan/Effects/HitEffectSystem.h"

#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/Physics/CoordinateSpace.h"

#include <algorithm>
#include <cmath>

namespace
{
    // FadeAlpha:
    // - 남은 수명 비율에 따라 alpha를 줄입니다.
    // - normalizedLife가 1이면 원래 alpha, 0이면 완전히 투명해집니다.
    unsigned char FadeAlpha(unsigned char alpha, float normalizedLife)
    {
        const float faded = static_cast<float>(alpha) * std::clamp(normalizedLife, 0.0f, 1.0f);
        return static_cast<unsigned char>(std::clamp(faded, 0.0f, 255.0f));
    }
}

namespace rjm
{
    void HitEffectSystem::EmitHit(const HitEvent& event)
    {
        // DamageType에 맞는 기본 색을 고릅니다.
        // 이 덕분에 화염탄, 폭발탄, 고정 피해가 생겼을 때 같은 HitEvent 구조로 색만 다르게 표현할 수 있습니다.
        const Color baseColor = ColorForDamage(event.damage);

        // 피해량이 클수록 파티클 속도와 크기를 조금 키웁니다.
        // 너무 작은 피해도 보이게 하고, 너무 큰 피해도 화면을 덮지 않도록 상한/하한을 둡니다.
        const float power = std::clamp(event.damageResult.appliedAmount / 20.0f, 0.7f, 2.2f);

        // 현재는 고정 7개 파티클을 방사형으로 생성합니다.
        // event.normal을 더해 "맞은 방향으로 튀는 느낌"을 살립니다.
        for (int i = 0; i < 7; ++i)
        {
            const float angle = (static_cast<float>(i) / 7.0f) * math::Pi * 2.0f;
            const float speed = (70.0f + static_cast<float>(i % 3) * 28.0f) * power;

            HitEffectParticle particle;
            particle.position = event.position;
            particle.velocity = {
                std::cos(angle) * speed + event.normal.x * 45.0f * power,
                std::sin(angle) * speed + event.normal.y * 45.0f * power
            };
            particle.maxLifetimeSeconds = 0.18f + static_cast<float>(i % 2) * 0.06f;
            particle.lifetimeSeconds = particle.maxLifetimeSeconds;
            particle.radius = 2.0f + power;
            particle.color = baseColor;
            SpawnParticle(particle);
        }
    }

    void HitEffectSystem::Update(float deltaSeconds)
    {
        if (deltaSeconds <= 0.0f)
        {
            return;
        }

        // 삭제가 잦은 작은 배열이므로 swap-remove 방식으로 죽은 파티클을 제거합니다.
        // 순서가 중요하지 않은 파티클에는 erase보다 비용이 적습니다.
        for (std::size_t i = 0; i < particles_.size();)
        {
            HitEffectParticle& particle = particles_[i];
            particle.lifetimeSeconds -= deltaSeconds;

            if (particle.lifetimeSeconds <= 0.0f)
            {
                particles_[i] = particles_.back();
                particles_.pop_back();
                continue;
            }

            // 파티클 위치를 속도만큼 이동합니다.
            particle.position.x += particle.velocity.x * deltaSeconds;
            particle.position.y += particle.velocity.y * deltaSeconds;

            // 짧은 지수 감속을 적용해 피격 파편이 빠르게 잦아들게 합니다.
            particle.velocity.x *= std::pow(0.08f, deltaSeconds);

            // y-up 좌표계이므로 중력은 y속도를 감소시키는 방향입니다.
            particle.velocity.y = particle.velocity.y * std::pow(0.15f, deltaSeconds) - 600.0f * deltaSeconds;
            ++i;
        }
    }

    void HitEffectSystem::Draw() const
    {
        for (const HitEffectParticle& particle : particles_)
        {
            // normalizedLife:
            // - 1이면 생성 직후, 0이면 사라지기 직전입니다.
            // - alpha와 반지름을 이 값에 맞춰 줄여 자연스럽게 사라지게 합니다.
            const float normalizedLife = particle.maxLifetimeSeconds > 0.0f
                ? particle.lifetimeSeconds / particle.maxLifetimeSeconds
                : 0.0f;

            Color color = particle.color;
            color.a = FadeAlpha(color.a, normalizedLife);

            DrawCircleV(
                // 내부 월드 좌표는 y-up이므로 Raylib 렌더 좌표로 변환해서 그립니다.
                CoordinateSpace::WorldToRender(particle.position),
                particle.radius * std::clamp(normalizedLife, 0.25f, 1.0f),
                color);
        }
    }

    void HitEffectSystem::Clear()
    {
        particles_.clear();
    }

    void HitEffectSystem::SpawnParticle(const HitEffectParticle& particle)
    {
        // 파티클이 너무 많이 쌓이면 가장 오래된 파티클을 버립니다.
        // 연사 무기나 폭발 이펙트가 많아져도 렌더 비용이 계속 증가하지 않게 하기 위한 상한입니다.
        if (particles_.size() >= maxParticles_)
        {
            particles_.erase(particles_.begin());
        }

        particles_.push_back(particle);
    }

    Color HitEffectSystem::ColorForDamage(const Damage& damage)
    {
        // 이 색은 임시 primitive 이펙트용입니다.
        // 나중에 hitEffectId 기반 이펙트 프리셋을 만들더라도 DamageType별 fallback 색으로 남길 수 있습니다.
        switch (damage.type)
        {
        case DamageType::Fire:
            return Color{ 255, 120, 50, 220 };
        case DamageType::Explosive:
            return Color{ 255, 210, 80, 230 };
        case DamageType::TrueDamage:
            return Color{ 180, 120, 255, 230 };
        case DamageType::Physical:
        default:
            return Color{ 255, 235, 160, 225 };
        }
    }
}

