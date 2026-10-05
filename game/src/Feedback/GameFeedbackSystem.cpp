// GameFeedbackSystem.cpp
// - 역경직과 카메라 흔들림을 실제로 갱신합니다.

#include "RecoilJumpMan/Feedback/GameFeedbackSystem.h"

#include "RecoilJumpMan/Math/VectorMath.h"

#include <algorithm>
#include <cmath>

namespace
{
    // ClampNonNegative:
    // - 시간이나 게이지처럼 0 아래로 내려가면 안 되는 값을 안전하게 자릅니다.
    // - 여기서는 remainingSeconds_가 아주 작은 음수로 떨어지는 부동소수점 오차를 막습니다.
    float ClampNonNegative(float value)
    {
        return std::max(0.0f, value);
    }
}

namespace rjm
{
    void HitStopController::Add(float seconds)
    {
        // 0 이하의 값은 "역경직 없음"으로 취급합니다.
        // 데이터 파일에서 기본값 0이 들어오는 경우를 안전하게 처리합니다.
        if (seconds <= 0.0f)
        {
            return;
        }

        // 새 역경직을 무조건 더하지 않고, 현재 남은 시간과 새 시간 중 큰 값을 선택합니다.
        // 이렇게 해야 연사 무기가 매 프레임 히트스톱을 쌓아 게임을 오래 멈추는 일을 막을 수 있습니다.
        // maxSingleStopSeconds_는 데이터 실수나 과도한 연출값을 막는 마지막 안전장치입니다.
        remainingSeconds_ = std::max(
            remainingSeconds_,
            std::min(seconds, maxSingleStopSeconds_));
    }

    float HitStopController::ConsumeGameplayDelta(float rawDeltaSeconds)
    {
        // 음수나 0초 프레임은 게임플레이도 진행할 수 없으므로 0을 반환합니다.
        if (rawDeltaSeconds <= 0.0f)
        {
            return 0.0f;
        }

        // 역경직이 없으면 원래 프레임 시간을 그대로 게임플레이에 넘깁니다.
        if (remainingSeconds_ <= 0.0f)
        {
            return rawDeltaSeconds;
        }

        // 남은 역경직이 이번 프레임 시간보다 길면, 이번 프레임의 게임플레이 시간은 완전히 0입니다.
        // 예: 0.05초 역경직 중 0.016초 프레임이면 플레이어/적/총알은 움직이지 않습니다.
        if (remainingSeconds_ >= rawDeltaSeconds)
        {
            remainingSeconds_ -= rawDeltaSeconds;
            return 0.0f;
        }

        // 남은 역경직이 이번 프레임 중간에 끝나는 경우입니다.
        // 예: 역경직 0.004초가 남았고 프레임이 0.016초라면, 앞 0.004초만 멈추고 뒤 0.012초는 진행합니다.
        const float gameplayDeltaSeconds = rawDeltaSeconds - remainingSeconds_;
        remainingSeconds_ = 0.0f;
        return gameplayDeltaSeconds;
    }

    bool HitStopController::IsActive() const
    {
        return remainingSeconds_ > 0.0f;
    }

    void CameraShakeController::Add(float strength, float seconds, float frequency)
    {
        // 강도나 시간이 0 이하라면 흔들림 효과가 없으므로 무시합니다.
        if (strength <= 0.0f || seconds <= 0.0f)
        {
            return;
        }

        // amplitude_는 흔들림의 세기입니다.
        // 여러 효과가 같은 순간 들어오면 어느 정도 누적되지만, maxAmplitude_를 넘지 않습니다.
        amplitude_ = std::min(maxAmplitude_, amplitude_ + strength);

        // 더 긴 흔들림이 이미 있으면 기존 지속 시간을 유지합니다.
        // 짧은 효과가 긴 효과를 갑자기 잘라먹지 않게 하기 위함입니다.
        durationSeconds_ = std::max(durationSeconds_, seconds);
        remainingSeconds_ = std::max(remainingSeconds_, seconds);

        // 더 빠른 진동이 들어오면 그 빈도를 사용합니다.
        // 권총의 짧은 탁, 샷건의 묵직한 쾅 같은 차이를 데이터로 줄 수 있습니다.
        frequency_ = std::max(frequency_, frequency);
    }

    void CameraShakeController::Update(float rawDeltaSeconds)
    {
        // 흔들림이 끝났거나 세기가 없다면 상태를 깨끗하게 초기화합니다.
        // offset_을 0으로 되돌리지 않으면 마지막 흔들림 위치에 화면이 살짝 남을 수 있습니다.
        if (remainingSeconds_ <= 0.0f || amplitude_ <= 0.0f)
        {
            amplitude_ = 0.0f;
            durationSeconds_ = 0.0f;
            remainingSeconds_ = 0.0f;
            offset_ = { 0.0f, 0.0f };
            return;
        }

        // 카메라 흔들림은 역경직 중에도 계속 보여야 하므로 rawDeltaSeconds로 시간이 흐릅니다.
        elapsedSeconds_ += rawDeltaSeconds;
        remainingSeconds_ = ClampNonNegative(remainingSeconds_ - rawDeltaSeconds);

        // normalizedRemaining은 1에서 시작해서 0으로 내려가는 값입니다.
        // 이것을 제곱하면 초반에는 강하고 끝으로 갈수록 빠르게 잦아드는 느낌이 납니다.
        const float normalizedRemaining = durationSeconds_ > 0.0f
            ? remainingSeconds_ / durationSeconds_
            : 0.0f;
        const float currentAmplitude = amplitude_ * normalizedRemaining * normalizedRemaining;

        // elapsedSeconds_ * frequency_는 지금 파형이 몇 번 진동했는지에 해당합니다.
        // 여기에 2PI를 곱하면 sin에 넣을 라디안 각도가 됩니다.
        const float phase = elapsedSeconds_ * frequency_ * math::Pi * 2.0f;

        // x와 y에 서로 다른 파형을 써서 대각선 직선 흔들림처럼 보이지 않게 합니다.
        // y축은 0.65배로 줄여서 위아래 멀미를 조금 낮춥니다.
        offset_ = {
            std::sin(phase) * currentAmplitude,
            std::sin(phase * 1.37f + 1.7f) * currentAmplitude * 0.65f
        };

        // 흔들림이 끝나는 프레임에 상태를 정리합니다.
        if (remainingSeconds_ <= 0.0f)
        {
            amplitude_ = 0.0f;
            durationSeconds_ = 0.0f;
            frequency_ = 30.0f;
            offset_ = { 0.0f, 0.0f };
        }
    }

    Vector2 CameraShakeController::Offset() const
    {
        return offset_;
    }

    void GameFeedbackSystem::EmitFire(const WeaponFeedbackProfile& feedback)
    {
        // 발사 피드백:
        // - 총을 쏘는 즉시 발생하는 손맛입니다.
        // - 아직 적에게 맞았는지와 관계없이, 방아쇠를 당긴 순간의 반응입니다.
        hitStop_.Add(feedback.fireHitStopSeconds);
        cameraShake_.Add(
            feedback.fireShakeStrength,
            feedback.fireShakeSeconds,
            feedback.fireShakeFrequency);
    }

    void GameFeedbackSystem::EmitHit(const WeaponFeedbackProfile& feedback)
    {
        // 명중 피드백:
        // - 탄환이 적, 보스, 벽, 방패 같은 대상에 실제로 맞았을 때 발생할 손맛입니다.
        // - Projectile 충돌 처리에서 HitEvent가 만들어진 뒤 호출됩니다.
        hitStop_.Add(feedback.hitStopSeconds);
        cameraShake_.Add(
            feedback.hitShakeStrength,
            feedback.hitShakeSeconds,
            feedback.hitShakeFrequency);
    }

    float GameFeedbackSystem::ConsumeGameplayDelta(float rawDeltaSeconds)
    {
        // Scene은 이 반환값을 플레이어/적/투사체 업데이트에 넘깁니다.
        // 역경직 중에는 0이 될 수 있고, 역경직이 없으면 rawDeltaSeconds 그대로입니다.
        return hitStop_.ConsumeGameplayDelta(rawDeltaSeconds);
    }

    void GameFeedbackSystem::UpdateVisuals(float rawDeltaSeconds)
    {
        // 시각 효과는 게임 세계가 멈춰도 계속 업데이트합니다.
        // 그래야 "멈칫"하는 순간에 카메라 흔들림이 렉처럼 멈추지 않고 살아 있습니다.
        cameraShake_.Update(rawDeltaSeconds);
    }

    Vector2 GameFeedbackSystem::CameraShakeOffset() const
    {
        return cameraShake_.Offset();
    }

    bool GameFeedbackSystem::IsHitStopActive() const
    {
        return hitStop_.IsActive();
    }
}

