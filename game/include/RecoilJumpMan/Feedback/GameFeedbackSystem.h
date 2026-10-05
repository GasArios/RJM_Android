#pragma once

// GameFeedbackSystem.h
// - 발사, 피격, 폭발 같은 게임플레이 이벤트에서 발생하는 손맛 효과를 모읍니다.
// - 현재는 역경직과 카메라 흔들림을 담당합니다.

#include "RecoilJumpMan/Data/WeaponDefinition.h"

#include <raylib.h>

namespace rjm
{
    // HitStopController:
    // - "역경직" 또는 "히트스톱"만 담당하는 작은 컨트롤러입니다.
    // - Sleep처럼 프로그램 전체를 멈추지 않고, 게임플레이에 넘길 delta time만 줄입니다.
    // - 그래서 화면 그리기, 카메라 흔들림, UI 같은 시각 효과는 계속 움직일 수 있습니다.
    class HitStopController
    {
    public:
        // Add:
        // - 새 역경직 시간을 추가합니다.
        // - 여러 이벤트가 겹치면 가장 긴 남은 시간을 우선합니다.
        // - 예: 0.03초 역경직 중에 0.08초 역경직이 들어오면 0.08초로 늘어납니다.
        // - 반대로 0.08초 역경직 중에 0.03초가 들어오면 이미 더 긴 멈춤이 있으므로 무시됩니다.
        void Add(float seconds);

        // ConsumeGameplayDelta:
        // - rawDeltaSeconds 중 역경직으로 멈춰야 하는 시간을 소비하고,
        //   이번 프레임에 실제 게임플레이가 진행할 시간을 반환합니다.
        // - 반환값이 0이면 플레이어, 총알, 적 같은 게임 세계가 이번 프레임에는 멈춥니다.
        // - 역경직이 프레임 중간에 끝나면 남은 시간만큼만 게임플레이가 진행됩니다.
        float ConsumeGameplayDelta(float rawDeltaSeconds);

        // IsActive:
        // - 아직 소비되지 않은 역경직 시간이 남아 있는지 확인합니다.
        // - 디버그 표시나 연출 분기에서 사용할 수 있습니다.
        bool IsActive() const;

    private:
        // 남아 있는 역경직 시간입니다.
        // 단위는 초입니다. 0.05f라면 약 50ms입니다.
        float remainingSeconds_ = 0.0f;

        // 한 번의 이벤트가 걸 수 있는 최대 역경직입니다.
        // 데이터 실수로 2초 같은 값이 들어와 게임이 멈춘 것처럼 느껴지는 상황을 막습니다.
        float maxSingleStopSeconds_ = 0.12f;
    };

    // CameraShakeController:
    // - 화면 흔들림만 담당하는 작은 컨트롤러입니다.
    // - 실제 카메라의 월드 target을 바꾸지 않고, 렌더링 때 더할 화면 픽셀 오프셋만 계산합니다.
    // - 이렇게 하면 화면은 흔들리지만 마우스 조준 좌표 계산은 덜 흔들립니다.
    class CameraShakeController
    {
    public:
        // Add:
        // - 화면 흔들림을 추가합니다.
        // - strength는 화면 픽셀 단위의 최대 흔들림으로 취급합니다.
        // - seconds는 흔들림이 사라지는 데 걸리는 시간입니다.
        // - frequency는 좌우/상하로 얼마나 빠르게 진동할지 정합니다.
        void Add(float strength, float seconds, float frequency);

        // Update:
        // - 흔들림은 역경직 중에도 보여야 하므로 raw delta로 갱신합니다.
        void Update(float rawDeltaSeconds);

        Vector2 Offset() const;

    private:
        // 현재 흔들림의 최대 진폭입니다.
        // 여러 발사/피격 이벤트가 겹치면 Add에서 누적됩니다.
        float amplitude_ = 0.0f;

        // 현재 흔들림 이벤트의 전체 지속 시간입니다.
        // 남은 시간 비율을 계산해 흔들림을 자연스럽게 줄이는 데 사용합니다.
        float durationSeconds_ = 0.0f;

        // 현재 흔들림이 끝나기까지 남은 시간입니다.
        float remainingSeconds_ = 0.0f;

        // 흔들림 진동 빈도입니다.
        // 값이 크면 잘게 떨리고, 작으면 묵직하게 흔들립니다.
        float frequency_ = 30.0f;

        // 흔들림 파형 계산에 쓰는 누적 시간입니다.
        // 남은 시간이 아니라 전체 경과 시간을 써야 sin 파형이 이어집니다.
        float elapsedSeconds_ = 0.0f;

        // 이번 프레임에 카메라 렌더링에 더할 화면 픽셀 오프셋입니다.
        Vector2 offset_ = { 0.0f, 0.0f };

        // 누적 흔들림의 최대 상한입니다.
        // 핸드캐논, 폭발, 피격이 동시에 터져도 화면이 과하게 튀지 않게 막습니다.
        float maxAmplitude_ = 24.0f;
    };

    // GameFeedbackSystem:
    // - 게임플레이 이벤트와 실제 연출 컨트롤러 사이의 입구입니다.
    // - GameplayScene은 이 클래스에 "발사했다", "맞았다"만 알려주고,
    //   역경직과 카메라 흔들림의 세부 계산은 내부 컨트롤러가 처리합니다.
    class GameFeedbackSystem
    {
    public:
        // EmitFire:
        // - 총 발사 성공 순간의 피드백을 발생시킵니다.
        void EmitFire(const WeaponFeedbackProfile& feedback);

        // EmitHit:
        // - 명중 순간의 피드백을 발생시킵니다.
        // - 현재 전투 충돌은 아직 없지만, 구조를 미리 열어둡니다.
        void EmitHit(const WeaponFeedbackProfile& feedback);

        // ConsumeGameplayDelta:
        // - 이번 프레임 게임플레이 시스템이 사용할 delta를 반환합니다.
        float ConsumeGameplayDelta(float rawDeltaSeconds);

        // UpdateVisuals:
        // - 카메라 흔들림처럼 역경직 중에도 움직여야 하는 효과를 갱신합니다.
        void UpdateVisuals(float rawDeltaSeconds);

        // CameraShakeOffset:
        // - GameCamera에 전달할 최종 화면 흔들림 오프셋입니다.
        Vector2 CameraShakeOffset() const;

        // IsHitStopActive:
        // - 현재 역경직이 진행 중인지 확인합니다.
        bool IsHitStopActive() const;

    private:
        // 게임플레이 delta time을 줄이는 역경직 컨트롤러입니다.
        HitStopController hitStop_;

        // 카메라 렌더 오프셋을 만드는 화면 흔들림 컨트롤러입니다.
        CameraShakeController cameraShake_;
    };
}

