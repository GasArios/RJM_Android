#pragma once

// GameplayScene.h
// - 실제 플레이 화면을 표현하는 Scene입니다.
// - 현재는 플레이어, 레벨, HUD, 디버그 오버레이를 가지고 있습니다.

#include "RecoilJumpMan/Camera/GameCamera.h"
#include "RecoilJumpMan/Combat/AimAssistResolver.h"
#include "RecoilJumpMan/Combat/CombatHitResolver.h"
#include "RecoilJumpMan/Combat/CombatAutoAimResolver.h"
#include "RecoilJumpMan/Combat/HitEvent.h"
#include "RecoilJumpMan/Combat/PlayerHitResolver.h"
#include "RecoilJumpMan/Combat/PlayerTerrainEffectResolver.h"
#include "RecoilJumpMan/Combat/ProjectilePool.h"
#include "RecoilJumpMan/Combat/ProjectileWorldCollisionResolver.h"
#include "RecoilJumpMan/Core/InputFocus.h"
#include "RecoilJumpMan/Debug/DebugOverlay.h"
#include "RecoilJumpMan/Effects/HitEffectSystem.h"
#include "RecoilJumpMan/Entity/EnemyPool.h"
#include "RecoilJumpMan/Feedback/GameFeedbackSystem.h"
#include "RecoilJumpMan/Player/Player.h"
#include "RecoilJumpMan/Scene/Scene.h"
#include "RecoilJumpMan/UI/Hud.h"
#include "RecoilJumpMan/UI/TrajectoryPreview.h"
#include "RecoilJumpMan/World/Level.h"
#include "RecoilJumpMan/World/WorldInteractionResolver.h"
#include "RecoilJumpMan/Mobile/MobileAim.h"
#include "RecoilJumpMan/Mobile/ReloadQueue.h"

#include <cstddef>
#include <optional>
#include <vector>

namespace rjm
{
    class Gun;
    struct WeaponDefinition;

    // final:
    // - 이 클래스는 더 이상 상속하지 않겠다는 뜻입니다.
    // - 지금은 GameplayScene을 마지막 자식 씬으로 취급합니다.
    class GameplayScene final : public Scene
    {
    public:
        // override:
        // - 부모 클래스의 virtual 함수를 자식 클래스가 재정의한다는 뜻입니다.
        // - 오타나 함수 시그니처 실수를 컴파일러가 잡아줍니다.
        void OnEnter(GameContext& context) override;

        // 플레이 중 매 프레임 게임 로직을 갱신합니다.
        void Update(GameContext& context, float deltaSeconds) override;

        // 플레이 중 매 프레임 화면을 그립니다.
        void Draw(GameContext& context) const override;

    private:
        bool UpdateMobile(GameContext& context, float deltaSeconds);
        void DrawMobile() const;
        bool SaveMobileCheckpoint() const;
        void LoadMobileCheckpoint();
        mobile::MobileAim mobileAim_;
        mobile::ReloadQueue mobileReload_;
        bool mobileAssist_ = true, mobileShake_ = true;
        float mobileAutoSaveSeconds_ = 0;
        int mobilePreviousMenu_ = 0;
        std::string mobileNotice_;
        enum class PlayerLifeState
        {
            Alive,
            GameOver
        };

        enum class PlayerRecoveryReason
        {
            OutOfWorld,
            Death
        };

        // HandleGameplayInput:
        // - 무기 선택, 재장전, 발사 모드 전환, 발사 입력을 처리합니다.
        void HandleGameplayInput(GameContext& context);

        // UpdateGameOver:
        // - 임시 육체가 붕괴한 뒤 부활 앵커에서 재구성되기 전까지의 짧은 대기 상태를 갱신합니다.
        void UpdateGameOver(GameContext& context, float rawDeltaSeconds);

        // ResolveGameplayDelta:
        // - raw delta를 게임플레이 상한과 hitstop에 통과시켜 실제 게임 세계가 진행할 시간을 구합니다.
        float ResolveGameplayDelta(float rawDeltaSeconds);

        // UpdatePlayerPhysics:
        // - 플레이어 이동/충돌을 필요하면 여러 substep으로 나눠 갱신합니다.
        void UpdatePlayerPhysics(float rawDeltaSeconds, float gameplayDeltaSeconds);

        // UpdateWorldSystems:
        // - 투사체, 적, 레벨처럼 플레이어 외의 게임플레이 시스템을 갱신합니다.
        void UpdateWorldSystems(float gameplayDeltaSeconds);

        // UpdateWorldInteraction:
        // - 문/부활 앵커처럼 월드에 찍힌 상호작용 대상을 처리합니다.
        void UpdateWorldInteraction(GameContext& context);

        // HandleTransition:
        // - 전환 트리거가 가리키는 월드 앵커로 플레이어를 이동합니다.
        void HandleTransition(const TransitionTrigger& transition);

        // ResolveMouseWorldPosition:
        // - 현재 마우스 위치를 월드 좌표로 바꿉니다.
        // - 마우스가 게임 viewport 밖이면 플레이어 위치를 반환합니다.
        Vector2 ResolveMouseWorldPosition(GameContext& context, bool mouseInsideGameViewport) const;

        // UpdateGameplayCamera:
        // - 화면 흔들림 오프셋과 카메라 추적을 갱신합니다.
        void UpdateGameplayCamera(GameContext& context, float rawDeltaSeconds, Vector2 mouseWorldPosition);

        // UpdateTrajectoryPreview:
        // - 현재 조준 방향 기준 반동 이동 궤적 미리보기를 갱신합니다.
        void UpdateTrajectoryPreview(
            Vector2 mouseWorldPosition,
            Vector2 mouseScreenPosition,
            bool mouseInsideGameViewport,
            bool combatLockRequested);

        // ResolveAimAssist:
        // - 원래 마우스 월드 좌표와 현재 무기 데이터를 기준으로 이번 프레임의 조준 보정 결과를 계산합니다.
        // - 실제 마우스 좌표는 움직이지 않고, 발사/궤적에 사용할 목표 좌표만 바꿉니다.
        AimAssistResult ResolveAimAssist(
            Vector2 rawWorldTarget,
            Vector2 mouseScreenPosition,
            const Gun* currentGun,
            bool combatLockRequested);

        // CommitAimAssistResult:
        // - 계산된 조준 보정 결과를 이번 프레임 표시 상태와 락온 유지 상태로 확정합니다.
        // - ResolveAimAssist가 계산만 맡고, 상태 변경은 이 함수가 담당하게 해 호출 순서 꼬임을 줄입니다.
        void CommitAimAssistResult(const AimAssistResult& result, bool combatLockRequested);

        // BuildAimAssistCandidates:
        // - 현재 active 상태인 적들을 조준 보정 후보 목록으로 변환합니다.
        // - 후보 생성과 후보 선택을 분리해두면 보스 약점, 파괴 오브젝트, 아군 제외 규칙을 이 단계에 추가할 수 있습니다.
        void BuildAimAssistCandidates(std::vector<AimAssistCandidate>& outCandidates) const;

        // ApplyAimPredictionToCandidates:
        // - 움직이는 적에게 현재 위치가 아니라 약한 예측 조준점을 넣어 조준 보정이 예측 사격을 방해하지 않게 합니다.
        // - 좌클릭 일반 보정은 약하게, 우클릭 락온은 조금 더 강하게 적용합니다.
        void ApplyAimPredictionToCandidates(const WeaponDefinition& weapon, bool combatLockRequested);

        // DrawAimAssistIndicator:
        // - 조준 보정이 걸린 대상을 시각적으로 표시합니다.
        // - 현재는 작은 링만 그리지만, 나중에 전용 타깃 스프라이트나 접근성 색상 옵션으로 바꿀 수 있습니다.
        void DrawAimAssistIndicator() const;

        // DrawCombatLockAimLine:
        // - 우클릭 락온 중 탄환이 향할 방향을 붉은 점선으로 표시합니다.
        // - 반동 이동 궤적과 구분하기 위해 projectileWorldTarget을 기준으로 그립니다.
        void DrawCombatLockAimLine() const;

        // DrawDashedWorldLine:
        // - 월드 좌표 두 점 사이를 카메라 안에서 점선으로 그립니다.
        void DrawDashedWorldLine(Vector2 fromWorld, Vector2 toWorld, Color color, float dashLength, float gapLength, float thickness) const;

        // ResetRecoveryState:
        // - 씬 진입, 레벨 전환, 플레이어 재배치 시 복구 관련 임시 상태를 초기화합니다.
        void ResetRecoveryState();

        // UpdateLastSafePosition:
        // - 플레이어가 땅에 안정적으로 서 있는 시간을 누적하고,
        //   0.2초 이상 안정 상태가 유지되면 마지막 안전 위치를 갱신합니다.
        void UpdateLastSafePosition(float deltaSeconds);

        // RecoverIfOutOfWorld:
        // - 플레이어가 월드 아래로 과하게 빠졌는지 확인하고, 필요하면 안전 위치로 복구합니다.
        void RecoverIfOutOfWorld();

        // HandlePlayerDeath:
        // - 플레이어 체력이 0이 된 순간 게임오버 상태로 전환합니다.
        void HandlePlayerDeath();

        // RecoverPlayer:
        // - 복구 사유에 맞는 위치를 고르고 플레이어/카메라 상태를 재배치합니다.
        void RecoverPlayer(PlayerRecoveryReason reason);

        // ResolveRecoveryPosition:
        // - 최근 안전 위치, LDtk SafePoint, PlayerStart, 디버그 기본 위치 순서로 복구 위치를 고릅니다.
        Vector2 ResolveRecoveryPosition() const;

        // ResolveRespawnPosition:
        // - HP 0 사망 후 사용할 부활 앵커 위치를 고릅니다.
        Vector2 ResolveRespawnPosition() const;

        // DrawGameOverOverlay:
        // - 부활 앵커에서 몸을 재구성하는 동안 간단한 게임오버 표시를 그립니다.
        void DrawGameOverOverlay() const;

        // DrawInteractionPrompt:
        // - 현재 가까운 상호작용 대상이 있으면 간단한 입력 안내를 그립니다.
        void DrawInteractionPrompt() const;

        // 현재 레벨입니다. 바닥, 타일맵, 방 정보를 가집니다.
        Level level_;

        // 플레이어 캐릭터입니다.
        Player player_;

        // 적은 SpawnPoint 기반으로 미리 만들어두고, 플레이어 주변에서만 활성화합니다.
        EnemyPool enemies_;

        // 총알은 고정 크기 풀에서 재사용합니다.
        ProjectilePool projectiles_;

        // 투사체와 적의 충돌/피해 적용을 해석하고 HitEvent를 만듭니다.
        CombatHitResolver combatHitResolver_;

        // 적 접촉 공격과 플레이어 피격을 해석하고 HitEvent를 만듭니다.
        PlayerHitResolver playerHitResolver_;

        // 타일 효과와 플레이어 피격/복구/환경 modifier를 해석합니다.
        PlayerTerrainEffectResolver playerTerrainEffectResolver_;

        // 월드의 문/부활 앵커 같은 상호작용 후보를 해석합니다.
        WorldInteractionResolver worldInteractionResolver_;

        // 투사체와 solid 타일의 충돌을 해석합니다.
        // 적 충돌보다 먼저 실행해 벽 뒤의 적에게 총알이 닿지 않게 합니다.
        ProjectileWorldCollisionResolver projectileWorldCollisionResolver_;

        // 마우스 포인터 주변의 적 후보를 골라 발사 목표를 보정합니다.
        AimAssistResolver aimAssist_;

        // 우클릭 홀드 중 전투용 자동 락온 대상을 고릅니다.
        CombatAutoAimResolver combatAutoAim_;

        // 전투용 자동 락온 튜닝입니다.
        // 옵션/난이도 설정이 생기면 이 값을 교체하면 됩니다.
        CombatAutoAimTuning combatAutoAimTuning_;

        // 이번 프레임 조준 보정 후보 목록입니다.
        // 매 프레임 재사용해서 불필요한 임시 vector 생성을 줄입니다.
        std::vector<AimAssistCandidate> aimAssistCandidates_;

        // 이번 프레임의 조준 보정 결과입니다.
        // 실제 발사와 궤적 미리보기, 타깃 표시가 같은 결과를 읽도록 저장합니다.
        AimAssistResult currentAimAssist_;

        // 우클릭 락온이 유지 중인 대상 식별자입니다.
        // 현재는 EnemyPool의 enemy index를 저장하고, 나중에 EntityId로 바꿀 수 있습니다.
        std::size_t lockedAimTargetStableId_ = 0;
        bool hasLockedAimTarget_ = false;

        // 한 프레임 동안 발생한 명중 이벤트입니다.
        std::vector<HitEvent> hitEvents_;

        // 플레이어 물리 substep에서 발생한 타일 피격 이벤트입니다.
        std::vector<HitEvent> terrainHitEvents_;

        // 이번 프레임 플레이어 주변의 월드 상호작용 상태입니다.
        WorldInteractionResult currentWorldInteraction_;

        // 플레이어를 따라다니는 메트로베니아용 카메라입니다.
        GameCamera camera_;

        // 발사/명중/폭발 같은 이벤트의 역경직과 화면 흔들림을 관리합니다.
        GameFeedbackSystem feedback_;

        // 명중 이벤트를 시각 이펙트로 표현합니다.
        HitEffectSystem hitEffects_;

        // 현재 입력 focus입니다.
        // 나중에 인벤토리/지도/설정창 UI가 열리면 이 값을 바꿔 플레이 조작과 궤적 미리보기를 잠급니다.
        // 예: inputFocus_ = InputFocus::Inventory가 되면 좌클릭은 발사가 아니라 인벤토리 선택으로 해석됩니다.
        InputFocus inputFocus_ = InputFocus::Gameplay;

        // 플레이어의 생명/게임오버 상태입니다.
        PlayerLifeState playerLifeState_ = PlayerLifeState::Alive;

        // 게임오버 상태에서 부활 앵커로 이동하기 전까지 남은 시간입니다.
        float gameOverRemainingSeconds_ = 0.0f;
        float gameOverRecoverDelaySeconds_ = 0.65f;

        // 현재 등록된 부활 앵커 위치입니다.
        // 저장 지점/마을/캠프 상호작용이 생기면 이 값을 갱신합니다.
        std::optional<Vector2> boundRespawnPosition_;

        // 현재 마우스 방향으로 한 발을 쐈을 때의 이동 예상 경로 시스템입니다.
        // Simulator는 경로 데이터를 만들고, Path는 계산된 샘플을 저장하고, Renderer는 그 샘플을 화면에 그립니다.
        // 이렇게 세 역할을 나눠두면 나중에 선/점 대신 스프라이트 에셋으로 표시해도 계산 코드는 유지할 수 있습니다.
        TrajectoryPreviewSimulator trajectoryPreview_;
        TrajectoryPreviewPath trajectoryPreviewPath_;
        TrajectoryPreviewRenderer trajectoryPreviewRenderer_;

        // 체력과 탄약 같은 플레이 정보를 그리는 HUD입니다.
        Hud hud_;

        // FPS와 시간 같은 개발용 정보를 그리는 오버레이입니다.
        DebugOverlay debugOverlay_;

        // 실제 시간(raw time)이 아니라, clamp와 hitstop을 통과한 게임플레이 누적 시간입니다.
        // 적 리스폰, 보스 패턴처럼 게임 세계 안에서만 흘러야 하는 시간 기준으로 사용합니다.
        double gameplayTimeSeconds_ = 0.0;

        // 마지막으로 0.2초 이상 안정적으로 서 있던 안전 위치입니다.
        std::optional<Vector2> lastSafePosition_;

        // 현재 안정적으로 grounded 상태를 유지한 누적 시간입니다.
        // 착지한 바로 그 순간을 안전 위치로 저장하지 않기 위해 짧은 지연을 둡니다.
        float stableGroundedSeconds_ = 0.0f;
    };
}
