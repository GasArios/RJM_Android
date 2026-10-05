#pragma once

// EnemyAi.h
// - 일반 필드 몬스터가 공유하는 데이터 주도형 AI 축과 런타임 상태입니다.
// - 몬스터마다 별도 클래스를 만들기보다 EnemyDefinition의 EnemyAiProfile 값으로 성향을 조합합니다.

#include <raylib.h>

namespace rjm
{
    enum class EnemyApproachStyle
    {
        None,
        Walk,
        Run,
        Hop,
        ClimbWall,
        Fly
    };

    enum class EnemyAttackStyle
    {
        Contact,
        Lunge,
        JumpPounce,
        Projectile,
        Area,
        Grab,
        Shield
    };

    enum class EnemyPressureTarget
    {
        PlayerCurrentPosition,
        LastGroundPosition,
        PredictedLandingPosition,
        ReloadSpot,
        Platform,
        ChokePoint
    };

    enum class EnemyReactionSpeed
    {
        Slow,
        Normal,
        Fast
    };

    enum class EnemyTelegraphProfile
    {
        LongStrong,
        Readable,
        ShortWeak
    };

    enum class EnemyRecoveryProfile
    {
        LargeOpening,
        Normal,
        SmallOpening
    };

    enum class EnemyAntiAirLevel
    {
        Ignore,
        Weak,
        Strong
    };

    enum class EnemyAiState
    {
        // Idle:
        // - 적이 멈춰서 플레이어를 찾거나 다음 배회 타이밍을 기다리는 상태입니다.
        Idle,

        // Wander:
        // - 플레이어를 보지 못하는 active 적이 스폰 주변을 가볍게 배회하는 상태입니다.
        Wander,

        // Alert:
        // - 플레이어를 발견한 직후 반응 시간입니다.
        Alert,

        // Approach:
        // - 플레이어 또는 마지막 목격 위치를 향해 접근하는 상태입니다.
        Approach,

        // Telegraph:
        // - 공격 전 예고 동작입니다.
        Telegraph,

        // Attack:
        // - 실제 공격이 활성화되는 상태입니다.
        Attack,

        // Recover:
        // - 공격 후 빈틈 상태입니다.
        Recover,

        // Return:
        // - leashRange 바깥으로 벗어나 스폰 위치로 돌아가는 상태입니다.
        Return
    };

    struct EnemyAiContext
    {
        Vector2 playerWorldPosition = { 0.0f, 0.0f };
        Vector2 playerWorldVelocity = { 0.0f, 0.0f };
        bool playerGrounded = false;
    };

    struct EnemyAiProfile
    {
        EnemyApproachStyle approachStyle = EnemyApproachStyle::Walk;
        EnemyAttackStyle attackStyle = EnemyAttackStyle::Contact;
        EnemyPressureTarget pressureTarget = EnemyPressureTarget::PlayerCurrentPosition;
        EnemyReactionSpeed reactionSpeed = EnemyReactionSpeed::Normal;
        EnemyTelegraphProfile telegraphProfile = EnemyTelegraphProfile::Readable;
        EnemyRecoveryProfile recoveryProfile = EnemyRecoveryProfile::Normal;
        EnemyAntiAirLevel antiAirLevel = EnemyAntiAirLevel::Ignore;

        // true이면 감지에 타일맵 Line of Sight를 요구합니다.
        // false이면 벽 너머 감지, 소리 감지, 마법 감지 같은 특수 적으로 취급할 수 있습니다.
        bool requiresLineOfSight = true;

        // true이면 플레이어를 보지 못하는 active 상태에서 스폰 주변을 배회합니다.
        bool canWander = true;

        // 플레이어 감지 거리입니다.
        float detectRange = 620.0f;

        // 스폰 위치에서 이 거리보다 멀어지면 Return 상태로 돌아갑니다.
        float leashRange = 1100.0f;

        // 접근 중 이 거리 안에 들어오면 더 전진하지 않는 선호 거리입니다.
        float preferredRange = 42.0f;

        // 공격을 시작할 수 있는 거리입니다.
        float attackRange = 54.0f;

        // 플레이어 시야가 끊긴 뒤 마지막 목격 위치를 기억하는 시간입니다.
        float pursuitMemorySeconds = 1.15f;

        // 기본 걷기 접근 속도 배율입니다.
        float moveSpeedMultiplier = 1.0f;

        // 뛰기 접근 속도 배율입니다.
        float runSpeedMultiplier = 1.45f;

        // 배회 속도 배율입니다. 보통 추적 속도보다 낮게 둡니다.
        float wanderSpeedMultiplier = 0.38f;

        // 스폰 지점 기준 배회 허용 반경입니다.
        float wanderRadius = 150.0f;

        // 한 번 배회 상태에 머무르는 시간입니다.
        float wanderSeconds = 1.15f;

        // Idle에서 다음 배회를 시작하기 전 대기 시간입니다.
        float idleSeconds = 0.75f;

        float attackCooldownSeconds = 1.3f;
        float attackActiveSeconds = 0.22f;
        float landingPredictionSeconds = 0.35f;

        float lungeSpeed = 300.0f;
        float jumpPounceHorizontalSpeed = 360.0f;
        float jumpPounceVerticalSpeed = 360.0f;
        float hopImpulseY = 190.0f;
        float hopIntervalSeconds = 0.8f;
    };
}

