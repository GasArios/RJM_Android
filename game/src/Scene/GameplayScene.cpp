// GameplayScene.cpp
// - 실제 게임 플레이 화면의 구현부입니다.
// - 현재는 시작 위치 설정, 시작 장비 장착, 플레이어/레벨 업데이트와 그리기를 담당합니다.

#include "RecoilJumpMan/Scene/GameplayScene.h"
#include "RecoilJumpMan/Mobile/WeaponInput.h"

#include "RecoilJumpMan/Combat/ProjectileHitPayload.h"
#include "RecoilJumpMan/Core/Config.h"
#include "RecoilJumpMan/Core/GameContext.h"
#include "RecoilJumpMan/Core/InputState.h"
#include "RecoilJumpMan/Data/DataRegistry.h"
#include "RecoilJumpMan/Math/VectorMath.h"
#include "RecoilJumpMan/Physics/CoordinateSpace.h"
#include "RecoilJumpMan/World/TileLineOfSight.h"
#include "RecoilJumpMan/World/TileMap.h"
#include "RecoilJumpMan/Mobile/TouchControls.h"

#include <raylib.h>

#include <algorithm>
#include <cmath>
#include <string>

namespace
{
    // MaxGameplayDeltaSeconds:
    // - 창 리사이즈, 디버거 중단, OS 멈춤 뒤에 큰 dt가 들어와도 게임 세계는 최대 1/30초만 진행합니다.
    // - 보스/적/투사체가 플레이어 입력 없이 몇 초치 행동을 한 번에 처리하는 억울한 상황을 막기 위한 상한입니다.
    constexpr float MaxGameplayDeltaSeconds = 1.0f / 30.0f;

    struct AimPredictionSettings
    {
        float weight = 0.0f;
        float maxSeconds = 0.0f;
        float maxDistance = 0.0f;
    };

    // 좌클릭 일반 보정:
    // - 플레이어가 적 앞쪽을 조준했을 때 보정이 현재 위치로 끌어내리지 않게 하는 약한 예측입니다.
    constexpr AimPredictionSettings ManualAimPrediction{
        0.45f,
        0.18f,
        70.0f
    };

    // 우클릭 전투 락온:
    // - 움직이는 적을 상대로 락온 신뢰성을 확보하기 위한 조금 더 강한 예측입니다.
    constexpr AimPredictionSettings CombatLockAimPrediction{
        0.75f,
        0.25f,
        120.0f
    };

    // ClampGameplayDeltaSeconds:
    // - 음수 dt는 0으로, 너무 큰 dt는 MaxGameplayDeltaSeconds로 자릅니다.
    float ClampGameplayDeltaSeconds(float rawDeltaSeconds)
    {
        return std::min(
            std::max(rawDeltaSeconds, 0.0f),
            MaxGameplayDeltaSeconds);
    }

    // PlayerPhysicsStepCount:
    // - 플레이어 충돌은 프레임이 느릴수록 더 잘게 쪼개 검사합니다.
    // - 기준은 raw dt입니다. 큰 dt가 들어오면 gameplay dt는 1/30초로 잘리지만,
    //   그 상황 자체가 불안정한 프레임이므로 4등분해서 더 촘촘히 검사합니다.
    int PlayerPhysicsStepCount(float rawDeltaSeconds)
    {
        if (rawDeltaSeconds <= 0.0f)
        {
            return 1;
        }

        if (rawDeltaSeconds <= 1.0f / 90.0f)
        {
            return 1;
        }

        if (rawDeltaSeconds <= 1.0f / 59.0f)
        {
            return 2;
        }

        if (rawDeltaSeconds <= 1.0f / 29.0f)
        {
            return 3;
        }

        return 4;
    }

    rjm::AimAssistResult BuildRawAimAssistResult(Vector2 rawWorldTarget)
    {
        rjm::AimAssistResult result;
        result.rawWorldTarget = rawWorldTarget;
        result.projectileWorldTarget = rawWorldTarget;
        result.recoilWorldTarget = rawWorldTarget;
        result.targetWorldPosition = rawWorldTarget;
        return result;
    }

    Vector2 ClampVectorLength(Vector2 value, float maxLength)
    {
        const float safeMaxLength = std::max(0.0f, maxLength);
        const float length = rjm::math::Length(value);
        if (length <= rjm::math::VectorEpsilon || length <= safeMaxLength)
        {
            return value;
        }

        return rjm::math::Scale(value, safeMaxLength / length);
    }
}

namespace rjm
{
    // OnEnter:
    // - GameplayScene에 들어올 때 한 번 호출됩니다.
    // - 현재는 플레이어의 시작 위치와 시작 장비를 설정합니다.
    void GameplayScene::OnEnter(GameContext& context)
    {
        // 플레이어 시작 위치를 지정합니다.
        // LDtk PlayerStart가 있으면 그 위치를 쓰고, 없으면 디버그 월드의 기본 바닥 위치를 사용합니다.
        const Vector2 playerStartPosition = level_.World().PlayerStartPosition()
            .value_or(Vector2{ 220.0f, level_.FloorWorldY() + player_.HalfBodySize() });
        player_.SetPosition(playerStartPosition);
        player_.SetVelocity({ 0.0f, 0.0f });
        ResetRecoveryState();
        boundRespawnPosition_ = level_.World().FirstRespawnPointPosition()
            .value_or(playerStartPosition);
        playerLifeState_ = PlayerLifeState::Alive;
        inputFocus_ = InputFocus::Gameplay;
        gameOverRemainingSeconds_ = 0.0f;
        gameplayTimeSeconds_ = 0.0;

        // 카메라가 맵 밖을 보여주지 않도록 레벨 경계를 넘겨줍니다.
        camera_.SetWorldBounds(level_.WorldBounds());
        camera_.SnapTo(player_.Position());

        // 총알 풀을 미리 잡아둡니다.
        // 이 용량을 넘으면 그 프레임의 새 총알은 생성되지 않습니다.
        projectiles_.Initialize(256);
        hitEvents_.clear();
        terrainHitEvents_.clear();
        hitEffects_.Clear();
        playerTerrainEffectResolver_.Reset();

        if (context.data)
        {
            // 실제 총기 수치는 BuiltInData.cpp에 있고, 씬은 시작 장비 id만 압니다.
            if (const WeaponDefinition* revolver = context.data->FindWeapon("old_revolver"))
            {
                player_.Weapons().Equip(0, Gun(*revolver));
            }

            if (const WeaponDefinition* shotgun = context.data->FindWeapon("hunter_shotgun"))
            {
                player_.Weapons().Equip(1, Gun(*shotgun));
            }

            if (const WeaponDefinition* handCannon = context.data->FindWeapon(
#ifdef __ANDROID__
                "recoil_smg"
#else
                "debug_hand_cannon"
#endif
            ))
            {
                player_.Weapons().Equip(2, Gun(*handCannon));
            }

            // 월드의 모든 SpawnPoint를 기반으로 적을 미리 만들어둡니다.
            if (const EnemyDefinition* debugEnemy = context.data->FindEnemy("debug_slime"))
            {
                enemies_.Initialize(level_.World(), *context.data, *debugEnemy);
            }
        }
#ifdef __ANDROID__
        camera_.SetZoomAnchored(mobile::MobileTuning::DefaultZoom,player_.Position());
        LoadMobileCheckpoint();
        mobile::Controls().SetAimMode(mobileAimMode_);
        camera_.SnapTo(player_.Position());
        TraceLog(LOG_INFO, "RJM: ready");
#endif
    }

    // Update:
    // - 매 프레임 플레이 화면의 로직을 갱신합니다.
    void GameplayScene::Update(GameContext& context, float deltaSeconds)
    {
#ifdef __ANDROID__
        if (!UpdateMobile(context, deltaSeconds)) return;
        deltaSeconds = std::min(deltaSeconds, 1.0f / 30.0f) * mobile::Controls().TimeScale();
#endif
        // rawDeltaSeconds:
        // - Raylib에서 실제로 흘렀다고 알려준 프레임 시간입니다.
        // - 역경직이 걸려도 이 값 자체는 바꾸지 않습니다.
        // - 카메라 흔들림, UI 애니메이션처럼 "멈춘 순간에도 움직여야 하는 것"은 이 시간을 씁니다.
        const float rawDeltaSeconds = deltaSeconds;

        if (playerLifeState_ == PlayerLifeState::GameOver)
        {
            UpdateGameOver(context, rawDeltaSeconds);
            return;
        }

        HandleGameplayInput(context);

        const float gameplayDeltaSeconds = ResolveGameplayDelta(rawDeltaSeconds);
        UpdatePlayerPhysics(rawDeltaSeconds, gameplayDeltaSeconds);
#ifdef __ANDROID__
        if (playerLifeState_ == PlayerLifeState::Alive) mobileReload_.Update(player_.Weapons(), player_.IsGrounded());
#endif
        if (playerLifeState_ == PlayerLifeState::GameOver)
        {
            hitEffects_.Update(rawDeltaSeconds);

            const bool mouseInsideGameViewport = context.input && context.input->MouseInsideGameViewport();
            const Vector2 mouseWorldPosition = ResolveMouseWorldPosition(context, mouseInsideGameViewport);
            UpdateGameplayCamera(context, rawDeltaSeconds, mouseWorldPosition);
            return;
        }

        UpdateWorldInteraction(context);
        if (playerLifeState_ == PlayerLifeState::GameOver)
        {
            return;
        }

        UpdateWorldSystems(gameplayDeltaSeconds);
        hitEffects_.Update(rawDeltaSeconds);

        const bool mouseInsideGameViewport = context.input && context.input->MouseInsideGameViewport();
        const Vector2 mouseScreenPosition = context.input
            ? context.input->MousePosition()
            : Vector2{ 0.0f, 0.0f };
        const bool combatLockRequested = context.input
            && AllowsGameplayControls(inputFocus_)
            && mouseInsideGameViewport
            && context.input->AimLockHeld();
        const Vector2 mouseWorldPosition = ResolveMouseWorldPosition(context, mouseInsideGameViewport);
        UpdateGameplayCamera(context, rawDeltaSeconds, mouseWorldPosition);
        UpdateTrajectoryPreview(mouseWorldPosition, mouseScreenPosition, mouseInsideGameViewport, combatLockRequested);
    }

    void GameplayScene::HandleGameplayInput(GameContext& context)
    {
        // context.input이 nullptr이 아니고, 현재 입력 focus가 Gameplay일 때만 플레이 입력을 처리합니다.
        // 포인터는 유효하지 않을 수 있으므로 사용 전에 검사하는 습관이 중요합니다.
        //
        // AllowsGameplayControls:
        // - 인벤토리, 지도, 설정창, 대화창이 열렸을 때 발사/재장전/무기 교체가 일어나지 않도록 막습니다.
        // - 지금은 inputFocus_가 항상 Gameplay이지만, UI가 생기면 이 조건이 실제로 플레이 조작 잠금 역할을 합니다.
        if (context.input
            && playerLifeState_ == PlayerLifeState::Alive
            && AllowsGameplayControls(inputFocus_))
        {
            player_.SetBracing(context.input->BraceHeld());

            // 마우스가 실제 창의 레터박스/장식 영역 위에 있으면 조준/발사는 처리하지 않습니다.
            // 키보드 기반 무기 선택이나 재장전은 그대로 허용합니다.
            const bool mouseInsideGameViewport = context.input->MouseInsideGameViewport();
            const bool combatLockRequested = mouseInsideGameViewport && context.input->AimLockHeld();

            // 이번 프레임에 선택된 무기 슬롯 번호를 가져옵니다.
            const int selectedSlot = context.input->SelectedWeaponSlot();

            // -1은 아무 슬롯도 선택하지 않았다는 뜻입니다.
            if (selectedSlot >= 0)
            {
                player_.Weapons().Select(selectedSlot);
            }

            // R 키가 눌리면 현재 총의 재장전을 시도합니다.
            // 실제로 가능한지는 Player/Gun 쪽에서 grounded 상태를 보고 판단합니다.
            // Manual 재장전은 탄창이 가득 차 있지 않다면, 남은 탄이 있어도 재장전을 시작할 수 있습니다.
            if (context.input->ReloadPressed())
            {
#ifdef __ANDROID__
                if (player_.Weapons().CountReloadTargets()>0) mobileReload_.Request();
#else
                player_.RequestReload(ReloadIntent::Manual);
#endif
            }

            // B 키로 현재 무기의 발사 모드를 전환합니다.
            // 지원 모드가 하나뿐인 무기는 CycleFireMode가 false를 반환하고 아무 일도 하지 않습니다.
            if (context.input->FireModeTogglePressed())
            {
                Gun* currentGun = player_.Weapons().Current();
                if (currentGun)
                {
                    currentGun->CycleFireMode();
                }
            }

            // 좌클릭 Pressed는 빈 탄창 재장전과 SemiAuto 발사에 사용합니다.
            // 좌클릭 Held는 FullAuto 발사에만 사용합니다.
            Gun* currentGun = player_.Weapons().Current();
            const bool firePressed = context.input->FirePressed() && mouseInsideGameViewport;

            if (currentGun)
            {
                // 발사 모드에 따라 이번 프레임에 발사를 원하는지 판단합니다.
                // SemiAuto는 클릭 순간만, FullAuto는 누르고 있는 동안 쿨타임마다 TryFireAt을 시도합니다.
#ifdef __ANDROID__
                const auto request=mobile::ResolveWeaponFire(player_.Weapons(),firePressed,
                    context.input->FireHeld() && mouseInsideGameViewport,mobile::IsPadMode(mobileAimMode_));
                currentGun=request.gun;
                const bool wantsFire=request.fire;
                if(request.switched) TraceLog(LOG_INFO,"RJM: auto switch slot=%d",player_.Weapons().CurrentSlot());
                if(request.reload) {
                    mobileReload_.Request();
                    mobile::Controls().BlockFireUntilRelease();
                    TraceLog(LOG_INFO,"RJM: reload all queued grounded=%d",player_.IsGrounded());
                }
#else
                bool wantsFire = currentGun->FireMode() == FireControlMode::FullAuto
                    ? (context.input->FireHeld() || firePressed) && mouseInsideGameViewport
                    : firePressed;

                if (wantsFire)
                {
                    // 현재 총이 비어 있으면 먼저 다음 발사 가능한 총으로 자동 교체합니다.
                    // 이렇게 하면 공중에서 한 총의 탄을 다 썼을 때도, 다른 총에 탄이 있으면 즉시 이어서 반동을 만들 수 있습니다.
                    if (currentGun->AmmoInMagazine() <= 0)
                    {
                        if (player_.Weapons().SelectNextReadyWeapon())
                        {
                            currentGun = player_.Weapons().Current();

                            // 새 총의 발사 모드를 기준으로 입력을 다시 해석합니다.
                            // 예를 들어 FullAuto 총에서 SemiAuto 총으로 자동 교체되었다면,
                            // 계속 누르고 있는 입력만으로 SemiAuto가 반복 발사되지 않게 합니다.
                            wantsFire = currentGun && currentGun->FireMode() == FireControlMode::FullAuto
                                ? context.input->FireHeld() && mouseInsideGameViewport
                                : firePressed;
                        }
                        else
                        {
                            // 발사 가능한 다음 총이 없고, 장착 총 전체 탄창도 비었다면 더 이상 반동을 만들 수 없습니다.
                            // 공중에서는 별도 강제 낙하를 하지 않아도 중력 때문에 자연스럽게 떨어집니다.
                            // 지상 또는 착지 직전 재장전 버퍼는 기존 RequestReload 흐름이 처리합니다.
                            if (firePressed && !player_.Weapons().HasAnyLoadedWeapon())
                            {
                                player_.RequestReload(ReloadIntent::EmptyMagazineOnly);
                            }

                            return;
                        }
                    }
                }
#endif
                if(wantsFire) {
                    if (!currentGun || !wantsFire)
                    {
                        return;
                    }

                    const Vector2 mouseScreenPosition = context.input->MousePosition();
                    const Vector2 rawWorldTarget = ResolveMouseWorldPosition(context, mouseInsideGameViewport);
#ifdef __ANDROID__
                    if (!mobileAim_.Valid()) return;
#endif
                    const AimAssistResult aim = ResolveAimAssist(rawWorldTarget, mouseScreenPosition, currentGun, combatLockRequested);
                    CommitAimAssistResult(aim, combatLockRequested);
                    const ShotResult shot = player_.TryFireAt(aim.projectileWorldTarget, aim.recoilWorldTarget);

                    if (shot.fired)
                    {
#ifdef __ANDROID__
                        TraceLog(LOG_INFO, "RJM: fire slot=%d ammo=%d", player_.Weapons().CurrentSlot(), currentGun->AmmoInMagazine());
#endif
                        // ShotResult는 한 번의 발사 결과입니다.
                        // 리볼버/볼트액션은 projectiles에 1개만 들어가고,
                        // 샷건 같은 PelletSpread 무기는 한 번 발사에 여러 펠릿이 들어갑니다.
                        // 반동, 탄약 소모, 발사 피드백은 이미 "발사 1회" 기준으로 처리되었으므로
                        // 여기서는 실제 투사체 생성만 목록 수만큼 반복합니다.
                        //
                        // 총알은 플레이어 중심에서 생성합니다.
                        // Player::Position() 자체가 중심 좌표이므로 별도의 보정이 필요 없습니다.
                        // 나중에 총구 위치나 무기 스프라이트가 생기면 spawn 위치를 별도로 계산할 수 있습니다.
                        for (const ShotProjectile& shotProjectile : shot.projectiles)
                        {
                            ProjectileHitPayload hitPayload;
                            hitPayload.damage = shotProjectile.damage;
                            hitPayload.feedback = shot.feedback;
                            hitPayload.hitEffectId = "bullet_hit";

                            projectiles_.Spawn(
                                player_.Position(),
                                shotProjectile.velocity,
                                hitPayload,
                                currentGun->Definition().id,
                                shotProjectile.lifetimeSeconds,
                                shotProjectile.hitRadius);
                        }

                        // Gun은 직접 카메라를 흔들지 않습니다.
                        // ShotResult에 담긴 피드백 데이터를 씬의 피드백 시스템에 넘겨 실제 역경직/흔들림을 발생시킵니다.
                        feedback_.EmitFire(shot.feedback);
                    }
                }
            }
        }
        else
        {
            player_.SetBracing(false);
        }
    }

    void GameplayScene::UpdateGameOver(GameContext& context, float rawDeltaSeconds)
    {
        player_.SetBracing(false);
        trajectoryPreviewPath_.Clear();
        currentAimAssist_ = AimAssistResult{};
        hasLockedAimTarget_ = false;
        currentWorldInteraction_ = WorldInteractionResult{};

        if (rawDeltaSeconds > 0.0f)
        {
            gameOverRemainingSeconds_ -= rawDeltaSeconds;
        }

        feedback_.ConsumeGameplayDelta(rawDeltaSeconds);
        feedback_.UpdateVisuals(rawDeltaSeconds);
        hitEffects_.Update(rawDeltaSeconds);

        if (gameOverRemainingSeconds_ <= 0.0f)
        {
            RecoverPlayer(PlayerRecoveryReason::Death);
            playerLifeState_ = PlayerLifeState::Alive;
            inputFocus_ = InputFocus::Gameplay;
        }

        const bool mouseInsideGameViewport = context.input && context.input->MouseInsideGameViewport();
        const Vector2 mouseWorldPosition = ResolveMouseWorldPosition(context, mouseInsideGameViewport);
        UpdateGameplayCamera(context, rawDeltaSeconds, mouseWorldPosition);
    }

    float GameplayScene::ResolveGameplayDelta(float rawDeltaSeconds)
    {
        // cappedDeltaSeconds:
        // - 실제 프레임이 오래 멈췄더라도 게임플레이는 최대 1/30초만 진행합니다.
        // - 이 값을 hitstop에 먼저 통과시켜 최종 gameplayDeltaSeconds를 얻습니다.
        const float cappedDeltaSeconds = ClampGameplayDeltaSeconds(rawDeltaSeconds);

        // 역경직은 게임플레이 시간만 줄이고, 카메라 흔들림은 실제 시간 기준으로 계속 움직입니다.
        // 예를 들어 역경직 중이면 gameplayDeltaSeconds는 0이 될 수 있지만,
        // rawDeltaSeconds로 움직이는 카메라 흔들림은 계속 갱신됩니다.
        const float gameplayDeltaSeconds = feedback_.ConsumeGameplayDelta(cappedDeltaSeconds);
        feedback_.UpdateVisuals(rawDeltaSeconds);
        return gameplayDeltaSeconds;
    }

    void GameplayScene::UpdatePlayerPhysics(float rawDeltaSeconds, float gameplayDeltaSeconds)
    {
        // 플레이어와 레벨의 상태를 갱신합니다.
        // 이 줄들에는 gameplayDeltaSeconds를 넣습니다.
        // 그래야 역경직 중에 플레이어, 총알, 적, 레벨 로직이 잠깐 멈춥니다.
        const int playerPhysicsSteps = PlayerPhysicsStepCount(rawDeltaSeconds);
        const float playerPhysicsDeltaSeconds = gameplayDeltaSeconds / static_cast<float>(playerPhysicsSteps);
        for (int step = 0; step < playerPhysicsSteps; ++step)
        {
            const TileMap& tileMap = level_.World().Map();
            const PlayerEnvironmentModifiers environment = playerTerrainEffectResolver_.BuildEnvironment(player_, tileMap);

            player_.Update(playerPhysicsDeltaSeconds, tileMap, environment);

            terrainHitEvents_.clear();
            const PlayerTerrainEffectResult terrainResult = playerTerrainEffectResolver_.ResolveAfterMovement(
                player_,
                tileMap,
                playerPhysicsDeltaSeconds,
                gameplayTimeSeconds_,
                terrainHitEvents_);

            for (const HitEvent& hitEvent : terrainHitEvents_)
            {
                feedback_.EmitHit(hitEvent.feedback);
                hitEffects_.EmitHit(hitEvent);
            }

            if (terrainResult.recoveryRequest == PlayerTerrainRecoveryRequest::SafePoint)
            {
                RecoverPlayer(PlayerRecoveryReason::OutOfWorld);
                playerTerrainEffectResolver_.Reset();
                continue;
            }

            HandlePlayerDeath();
            if (playerLifeState_ != PlayerLifeState::Alive)
            {
                return;
            }

            if (terrainResult.canRecordSafePosition)
            {
                UpdateLastSafePosition(playerPhysicsDeltaSeconds);
            }
            else
            {
                stableGroundedSeconds_ = 0.0f;
            }

            RecoverIfOutOfWorld();
            if (playerLifeState_ != PlayerLifeState::Alive)
            {
                return;
            }
        }
    }

    void GameplayScene::UpdateWorldSystems(float gameplayDeltaSeconds)
    {
        projectiles_.Update(gameplayDeltaSeconds);
        projectileWorldCollisionResolver_.ResolveProjectilesAgainstWorld(
            projectiles_,
            level_.World().Map());

        gameplayTimeSeconds_ += static_cast<double>(gameplayDeltaSeconds);
        enemies_.Update(
            level_.World(),
            player_.Position(),
            player_.Velocity(),
            player_.IsGrounded(),
            gameplayTimeSeconds_,
            gameplayDeltaSeconds);

        hitEvents_.clear();
        combatHitResolver_.ResolveProjectilesAgainstEnemies(
            projectiles_,
            enemies_,
            gameplayTimeSeconds_,
            hitEvents_);
        playerHitResolver_.ResolveEnemyContact(
            player_,
            enemies_,
            gameplayTimeSeconds_,
            hitEvents_);

        for (const HitEvent& hitEvent : hitEvents_)
        {
            feedback_.EmitHit(hitEvent.feedback);
            hitEffects_.EmitHit(hitEvent);
        }

        HandlePlayerDeath();

        level_.Update(gameplayDeltaSeconds);
    }

    void GameplayScene::UpdateWorldInteraction(GameContext& context)
    {
        currentWorldInteraction_ = WorldInteractionResult{};

        if (!context.input
            || playerLifeState_ != PlayerLifeState::Alive
            || !AllowsGameplayControls(inputFocus_))
        {
            return;
        }

        currentWorldInteraction_ = worldInteractionResolver_.Resolve(
            level_.World(),
            player_.Position(),
            context.input->InteractPressed());

        if (!currentWorldInteraction_.activated)
        {
            return;
        }

        if (currentWorldInteraction_.shouldBindRespawnPoint && currentWorldInteraction_.respawnPoint)
        {
            boundRespawnPosition_ = currentWorldInteraction_.respawnPoint->position;
        }

        if (currentWorldInteraction_.transition)
        {
            HandleTransition(*currentWorldInteraction_.transition);
        }
    }

    void GameplayScene::HandleTransition(const TransitionTrigger& transition)
    {
        if (transition.targetSpawnId.empty())
        {
            return;
        }

        const WorldAnchor* anchor = level_.World().FindAnchor(transition.targetSpawnId);
        if (!anchor)
        {
            return;
        }

        player_.SetPosition(anchor->position);
        player_.SetVelocity({ 0.0f, 0.0f });
        player_.SetBracing(false);
        ResetRecoveryState();
        playerTerrainEffectResolver_.Reset();
        camera_.SnapTo(anchor->position);
    }

    Vector2 GameplayScene::ResolveMouseWorldPosition(GameContext& context, bool mouseInsideGameViewport) const
    {
#ifdef __ANDROID__
        return mouseInsideGameViewport && mobileAim_.Valid() ? mobileAim_.Target(player_.Position()) : player_.Position();
#else
        return mouseInsideGameViewport && context.input
            ? camera_.ScreenToWorld(context.input->MousePosition())
            : player_.Position();
#endif
    }

    void GameplayScene::UpdateGameplayCamera(GameContext& context, float rawDeltaSeconds, Vector2 mouseWorldPosition)
    {
        const float mouseWheelMove = context.input ? context.input->MouseWheelMove() : 0.0f;

        // 카메라 업데이트는 rawDeltaSeconds를 씁니다.
        // 카메라 흔들림과 추적 보간은 역경직 중에도 화면에서 살아 있어야 하기 때문입니다.
        // SetScreenShakeOffset은 카메라의 실제 target이 아니라 렌더링 offset에만 영향을 줍니다.
        camera_.SetScreenShakeOffset(mobileShake_ ? feedback_.CameraShakeOffset() : Vector2{});
        camera_.Update(player_.Position(), player_.Velocity(), mouseWorldPosition, mouseWheelMove, rawDeltaSeconds);
    }

    void GameplayScene::UpdateTrajectoryPreview(
        Vector2 mouseWorldPosition,
        Vector2 mouseScreenPosition,
        bool mouseInsideGameViewport,
        bool combatLockRequested)
    {
        // 궤적 미리보기 요청을 만듭니다.
        // TrajectoryPreviewSimulator가 GameplayScene, Player, Level, Camera에 직접 의존하지 않도록
        // 이번 프레임 계산에 필요한 값만 구조체에 담아 넘깁니다.
        TrajectoryPreviewRequest previewRequest;

        // inputFocus:
        // - Gameplay이 아니면 Simulator가 빈 경로를 만들고 return합니다.
        // - 덕분에 가방/지도/설정창에서도 이전 프레임 궤적이 화면에 남지 않습니다.
        previewRequest.inputFocus = mouseInsideGameViewport ? inputFocus_ : InputFocus::Pause;

        // gun:
        // - 현재 선택한 총입니다.
        // - Simulator는 이 총의 PreviewShot을 호출해 탄약을 소모하지 않고 반동 벡터를 얻습니다.
        previewRequest.gun = player_.Weapons().Current();

        // currentAimAssist_:
        // - 궤적 미리보기와 실제 발사가 같은 보정 규칙을 쓰도록 여기서 한 번 계산합니다.
        // - 조준 보정이 꺼져 있거나 후보가 없으면 raw mouseWorldPosition이 그대로 들어갑니다.
        if (mouseInsideGameViewport)
        {
            const AimAssistResult aim = ResolveAimAssist(
                mouseWorldPosition,
                mouseScreenPosition,
                previewRequest.gun,
                combatLockRequested);
            CommitAimAssistResult(aim, combatLockRequested);
        }
        else
        {
            currentAimAssist_ = AimAssistResult{};
            hasLockedAimTarget_ = false;
        }

        // playerPosition / playerVelocity / playerGrounded:
        // - 실제 플레이어 현재 상태입니다.
        // - playerPosition은 중심 좌표입니다.
        // - Simulator는 이 값을 복사해서 가상 중심 위치/속도만 굴리므로 실제 Player는 움직이지 않습니다.
        previewRequest.playerPosition = player_.Position();
        previewRequest.playerVelocity = player_.Velocity();
        previewRequest.playerGrounded = player_.IsGrounded();
        previewRequest.playerAirborneStartY = player_.AirborneStartY();
        previewRequest.playerBracing = player_.IsBracing();

        // aimWorldTarget:
        // - 현재 마우스 위치를 카메라 기준으로 변환한 월드 좌표입니다.
        // - 이 값과 조준 원점의 차이가 "총알이 나가는 방향"이 됩니다.
        // - 궤적은 플레이어 몸이 어디로 밀릴지를 보여줘야 하므로 recoilWorldTarget을 사용합니다.
        previewRequest.aimWorldTarget = currentAimAssist_.recoilWorldTarget;

        // playerHalfSize:
        // - 궤적 시뮬레이터가 중심 좌표에서 바닥 접촉을 계산할 때 사용합니다.
        previewRequest.playerHalfSize = player_.HalfBodySize();

        // useGroundPlane / groundY:
        // - tileMap이 없거나 타일 충돌을 끄는 상황에서만 사용할 fallback 바닥입니다.
        // - 실제 미리보기는 아래 tileMap/useTileCollision 경로를 우선 사용합니다.
        previewRequest.useGroundPlane = true;
        previewRequest.groundY = level_.FloorWorldY();

        // 실제 플레이어 이동과 같은 TileCollisionResolver를 쓰도록 타일맵을 넘깁니다.
        // 이렇게 해야 궤적 미리보기가 벽/발판을 통과해서 거짓 경로를 보여주는 일을 줄일 수 있습니다.
        previewRequest.tileMap = &level_.World().Map();
        previewRequest.useTileCollision = true;

        // Build:
        // - player_.Movement()를 넘겨 실제 반동 이동 물리와 같은 공식으로 N초 미래를 시뮬레이션합니다.
        // - 결과 샘플들은 trajectoryPreviewPath_에 저장되고, Draw 단계에서 렌더러가 읽습니다.
        trajectoryPreview_.Build(previewRequest, player_.Movement(), trajectoryPreviewPath_);
    }

    AimAssistResult GameplayScene::ResolveAimAssist(
        Vector2 rawWorldTarget,
        Vector2 mouseScreenPosition,
        const Gun* currentGun,
        bool combatLockRequested)
    {
        // 무기가 없으면 조준 보정도 적용할 대상이 없습니다.
        // rawWorldTarget을 그대로 반환해 호출부가 별도 예외 처리를 하지 않아도 되게 합니다.
        if (!currentGun)
        {
            return BuildRawAimAssistResult(rawWorldTarget);
        }

        const WeaponDefinition& weapon = currentGun->Definition();

        // 후보 목록은 active 적에서 매번 새로 구성합니다.
        // 적이 죽거나 화면 밖으로 비활성화되면 다음 프레임 후보에서 자연스럽게 빠집니다.
        BuildAimAssistCandidates(aimAssistCandidates_);
        ApplyAimPredictionToCandidates(weapon, combatLockRequested);

        if (combatLockRequested)
        {
            CombatAutoAimTuning tuning = combatAutoAimTuning_;
            tuning.enabled = tuning.enabled && weapon.aimAssistEnabled;
            tuning.useLockTargetForRecoil = weapon.aimAssistUsesAssistedRecoil;
            tuning.usePredictedTarget = true;

            return combatAutoAim_.Resolve(
                player_.Position(),
                rawWorldTarget,
                aimAssistCandidates_,
                tuning,
                hasLockedAimTarget_,
                lockedAimTargetStableId_);
        }

        // 우클릭 락온이 아니면 기존 마우스 포인터 근처 보정으로 돌아갑니다.
        AimAssistTuning tuning;
        tuning.enabled = weapon.aimAssistEnabled && mobileAssist_;
        tuning.screenRadiusPixels = weapon.aimAssistRadiusPixels;
        tuning.maxCorrectionDegrees = weapon.aimAssistMaxCorrectionDegrees;
        tuning.useAssistedTargetForRecoil = weapon.aimAssistUsesAssistedRecoil;
#ifdef __ANDROID__
        tuning.useAssistedTargetForRecoil = false;
        tuning.maxCorrectionDegrees = std::min(tuning.maxCorrectionDegrees, 6.0f);
        tuning.directionOnly = mobileAimMode_!=mobile::AimMode::Character;
        tuning.viewportSize = camera_.ViewportSize();
        tuning.directionRangeWorld = weapon.projectileRange>0 ? weapon.projectileRange : 1000.0f;
#endif
        tuning.usePredictedTarget = true;

        return aimAssist_.Resolve(
            player_.Position(),
            rawWorldTarget,
            mouseScreenPosition,
            aimAssistCandidates_,
            tuning);
    }

    void GameplayScene::CommitAimAssistResult(const AimAssistResult& result, bool combatLockRequested)
    {
        currentAimAssist_ = result;

        if (combatLockRequested
            && result.assisted
            && result.lockOn
            && result.hasTargetStableId)
        {
            lockedAimTargetStableId_ = result.targetStableId;
            hasLockedAimTarget_ = true;
            return;
        }

        hasLockedAimTarget_ = false;
    }

    void GameplayScene::BuildAimAssistCandidates(std::vector<AimAssistCandidate>& outCandidates) const
    {
        outCandidates.clear();

        const std::vector<Enemy>& enemies = enemies_.Enemies();
        for (std::size_t enemyIndex : enemies_.ActiveIndices())
        {
            if (enemyIndex >= enemies.size())
            {
                continue;
            }

            const Enemy& enemy = enemies[enemyIndex];
            if (!enemy.AllowsAimAssist())
            {
                continue;
            }

            const Vector2 worldPoint = enemy.AimAssistPoint();

            AimAssistCandidate candidate;
            candidate.targetId = enemy.Definition().id;
            candidate.worldPosition = worldPoint;
            candidate.screenPosition = camera_.WorldToScreen(worldPoint);
            candidate.screenRadiusPixels = enemy.AimAssistRadiusPixels();
            candidate.priority = enemy.AimAssistPriority();
            candidate.enabled = true;
            candidate.stableId = enemyIndex;
            candidate.hasStableId = true;
            candidate.worldVelocity = enemy.Velocity();
            candidate.hasLineOfSight = TileLineOfSight::Trace(
                level_.World().Map(),
                player_.Position(),
                worldPoint).hasLineOfSight;
            candidate.predictedWorldPosition = candidate.worldPosition;
            candidate.predictedScreenPosition = candidate.screenPosition;
            candidate.predictedHasLineOfSight = candidate.hasLineOfSight;
            candidate.hasPredictedWorldPosition = false;
            outCandidates.push_back(candidate);
        }
    }

    void GameplayScene::ApplyAimPredictionToCandidates(const WeaponDefinition& weapon, bool combatLockRequested)
    {
        const AimPredictionSettings settings = combatLockRequested
            ? CombatLockAimPrediction
            : ManualAimPrediction;

        const float projectileSpeed = std::max(1.0f, weapon.bulletSpeed);
        if (settings.weight <= 0.0f || settings.maxSeconds <= 0.0f || settings.maxDistance <= 0.0f)
        {
            return;
        }

        const TileMap& tileMap = level_.World().Map();
        const Vector2 playerPosition = player_.Position();

        for (AimAssistCandidate& candidate : aimAssistCandidates_)
        {
            candidate.predictedWorldPosition = candidate.worldPosition;
            candidate.predictedScreenPosition = candidate.screenPosition;
            candidate.predictedHasLineOfSight = candidate.hasLineOfSight;
            candidate.hasPredictedWorldPosition = false;

            const float distance = math::Distance(playerPosition, candidate.worldPosition);
            const float travelSeconds = std::min(distance / projectileSpeed, settings.maxSeconds);
            Vector2 predictedOffset = math::Scale(candidate.worldVelocity, travelSeconds);
            predictedOffset = ClampVectorLength(predictedOffset, settings.maxDistance);
            predictedOffset = math::Scale(predictedOffset, std::clamp(settings.weight, 0.0f, 1.0f));

            if (math::Length(predictedOffset) <= math::VectorEpsilon)
            {
                continue;
            }

            const Vector2 predictedWorldPosition = math::Add(candidate.worldPosition, predictedOffset);
            const TileLineOfSightResult predictedLine = TileLineOfSight::Trace(
                tileMap,
                playerPosition,
                predictedWorldPosition);

            // 예측점이 벽 뒤라면 현재 위치 보정으로 되돌립니다.
            // 이렇게 해야 자동 예측이 벽 너머로 총알을 유도하는 느낌을 만들지 않습니다.
            if (!predictedLine.hasLineOfSight)
            {
                continue;
            }

            candidate.predictedWorldPosition = predictedWorldPosition;
            candidate.predictedScreenPosition = camera_.WorldToScreen(predictedWorldPosition);
            candidate.predictedHasLineOfSight = true;
            candidate.hasPredictedWorldPosition = true;
        }
    }

    void GameplayScene::DrawCombatLockAimLine() const
    {
        if (!currentAimAssist_.lockOn || !currentAimAssist_.assisted)
        {
            return;
        }

        const Vector2 startWorld = player_.Position();
        const Vector2 requestedEndWorld = currentAimAssist_.targetWorldPosition;
        const TileLineOfSightResult visualLineOfSight = TileLineOfSight::Trace(level_.World().Map(), startWorld, requestedEndWorld);
        const Vector2 visibleEndWorld = visualLineOfSight.hasLineOfSight
            ? requestedEndWorld
            : visualLineOfSight.visibleEndWorld;

        const Color lineColor = visualLineOfSight.hasLineOfSight
            ? Color{ 255, 74, 84, 215 }
            : Color{ 160, 52, 60, 150 };
        DrawDashedWorldLine(startWorld, visibleEndWorld, lineColor, 14.0f, 8.0f, 2.0f);

        // 시야선이 막혔을 때는 끊긴 지점에 작은 표시를 찍어 "적이 아니라 벽에서 막혔다"는 느낌을 줍니다.
        if (!visualLineOfSight.hasLineOfSight)
        {
            const Vector2 renderPosition = CoordinateSpace::WorldToRender(visibleEndWorld);
            DrawCircleV(renderPosition, 4.0f, Color{ 255, 105, 112, 180 });
        }
    }

    void GameplayScene::DrawDashedWorldLine(Vector2 fromWorld, Vector2 toWorld, Color color, float dashLength, float gapLength, float thickness) const
    {
        const Vector2 from = CoordinateSpace::WorldToRender(fromWorld);
        const Vector2 to = CoordinateSpace::WorldToRender(toWorld);
        const Vector2 delta = math::Subtract(to, from);
        const float length = math::Length(delta);
        if (length <= 0.0001f)
        {
            return;
        }

        const Vector2 direction = math::Scale(delta, 1.0f / length);
        const float safeDashLength = std::max(1.0f, dashLength);
        const float safeGapLength = std::max(0.0f, gapLength);

        for (float cursor = 0.0f; cursor < length; cursor += safeDashLength + safeGapLength)
        {
            const float segmentEnd = std::min(cursor + safeDashLength, length);
            const Vector2 start = {
                from.x + direction.x * cursor,
                from.y + direction.y * cursor
            };
            const Vector2 end = {
                from.x + direction.x * segmentEnd,
                from.y + direction.y * segmentEnd
            };
            DrawLineEx(start, end, thickness, color);
        }
    }

    void GameplayScene::DrawAimAssistIndicator() const
    {
        if (!currentAimAssist_.assisted)
        {
            return;
        }

        // currentAimAssist_는 월드 좌표를 담고 있으므로, 카메라 Begin 안에서 그릴 수 있는 렌더 좌표로 바꿉니다.
        // 이 표시는 실제 마우스 포인터를 움직이는 것이 아니라, "이번 발은 이 대상에게 보정됨"을 보여주는 보조 피드백입니다.
        const Vector2 renderPosition = CoordinateSpace::WorldToRender(currentAimAssist_.targetWorldPosition);
        const Color ringColor = currentAimAssist_.lockOn
            ? Color{ 255, 82, 92, 225 }
            : Color{ 120, 220, 255, 210 };
        const Color tickColor = currentAimAssist_.lockOn
            ? Color{ 255, 205, 210, 210 }
            : Color{ 230, 250, 255, 190 };
        const float radius = currentAimAssist_.lockOn ? 22.0f : 19.0f;

        DrawCircleLines(
            static_cast<int>(renderPosition.x),
            static_cast<int>(renderPosition.y),
            radius,
            ringColor);

        DrawLineV(
            { renderPosition.x - radius - 5.0f, renderPosition.y },
            { renderPosition.x - radius + 3.0f, renderPosition.y },
            tickColor);
        DrawLineV(
            { renderPosition.x + radius - 3.0f, renderPosition.y },
            { renderPosition.x + radius + 5.0f, renderPosition.y },
            tickColor);
        DrawLineV(
            { renderPosition.x, renderPosition.y - radius - 5.0f },
            { renderPosition.x, renderPosition.y - radius + 3.0f },
            tickColor);
        DrawLineV(
            { renderPosition.x, renderPosition.y + radius - 3.0f },
            { renderPosition.x, renderPosition.y + radius + 5.0f },
            tickColor);
    }

    // Draw:
    // - 매 프레임 플레이 화면을 그립니다.
    void GameplayScene::Draw(GameContext& context) const
    {
        // ClearBackground는 화면 전체를 지정한 색으로 지우는 Raylib 함수입니다.
        ClearBackground(Color{ 20, 22, 29, 255 });

        // 그리는 순서가 중요합니다.
        // 월드 오브젝트는 카메라 Begin/End 사이에 그리고,
        // HUD와 디버그 UI는 카메라 밖에서 그립니다.
        camera_.Begin();
        level_.Draw();
        enemies_.Draw();
        projectiles_.Draw();
        hitEffects_.Draw();

        // 궤적 미리보기는 월드 오브젝트이므로 카메라 Begin/End 사이에서 그립니다.
        // 현재는 선과 점으로 그리지만, Renderer가 분리되어 있어 나중에 에셋 기반 표시로 교체하기 쉽습니다.
        trajectoryPreviewRenderer_.Draw(trajectoryPreviewPath_);
        DrawCombatLockAimLine();
        DrawAimAssistIndicator();

        player_.Draw();
        camera_.End();

        hud_.Draw(player_);
        DrawInteractionPrompt();
        DrawGameOverOverlay();
#ifdef __ANDROID__
        DrawMobile();
#else
        debugOverlay_.Draw(context);
#endif
    }

    void GameplayScene::ResetRecoveryState()
    {
        // 씬에 처음 들어왔을 때는 플레이어가 아직 "안정적으로 서 있던 위치"를 증명하지 않았습니다.
        // 그래서 동적 안전 위치는 비우고, 필요하면 LDtk SafePoint/PlayerStart fallback을 사용합니다.
        lastSafePosition_.reset();
        stableGroundedSeconds_ = 0.0f;
    }

    void GameplayScene::UpdateLastSafePosition(float deltaSeconds)
    {
        if (deltaSeconds <= 0.0f)
        {
            return;
        }

        const Vector2 velocity = player_.Velocity();

        // 착지 순간에는 충돌 보정으로 위치가 맞춰졌더라도 아직 미끄러지거나 튕기는 중일 수 있습니다.
        // 너무 빠른 상태를 안전 위치로 저장하면 복구 직후 다시 떨어지는 위치가 될 수 있어 속도 조건을 둡니다.
        const bool stableVelocity = std::fabs(velocity.x) <= 80.0f
            && std::fabs(velocity.y) <= 5.0f;

        if (!player_.IsGrounded() || !stableVelocity)
        {
            stableGroundedSeconds_ = 0.0f;
            return;
        }

        stableGroundedSeconds_ += deltaSeconds;
        if (stableGroundedSeconds_ >= 0.2f)
        {
            // 0.2초 이상 땅에 안정적으로 머문 뒤에만 lastSafePosition을 갱신합니다.
            // 이 지연 덕분에 모서리에 살짝 닿은 순간이나 벽 속으로 보정되는 순간을 안전 위치로 저장하지 않습니다.
            lastSafePosition_ = player_.Position();
        }
    }

    void GameplayScene::RecoverIfOutOfWorld()
    {
        const WorldMap& world = level_.World();

        // 월드 바닥보다 3타일 아래로 내려가면 정상 플레이 영역을 벗어난 것으로 봅니다.
        // 한두 픽셀의 충돌 보정 흔들림은 허용하고, 실제 낙하/끼임 상황만 복구하기 위한 여유값입니다.
        const float recoveryY = world.WorldBounds().y - static_cast<float>(world.Map().TileSize() * 3);

        if (player_.Position().y >= recoveryY)
        {
            return;
        }

        RecoverPlayer(PlayerRecoveryReason::OutOfWorld);
    }

    void GameplayScene::HandlePlayerDeath()
    {
        if (playerLifeState_ != PlayerLifeState::Alive || !player_.IsDead())
        {
            return;
        }

        playerLifeState_ = PlayerLifeState::GameOver;
#ifdef __ANDROID__
        mobile::Controls().BlockFireUntilRelease();
        mobileReload_ = mobile::ReloadQueue{};
#endif
        inputFocus_ = InputFocus::Pause;
        gameOverRemainingSeconds_ = gameOverRecoverDelaySeconds_;
        player_.SetVelocity({ 0.0f, 0.0f });
        player_.SetBracing(false);
        stableGroundedSeconds_ = 0.0f;
    }

    void GameplayScene::RecoverPlayer(PlayerRecoveryReason reason)
    {
        const Vector2 recoveryPosition = reason == PlayerRecoveryReason::Death
            ? ResolveRespawnPosition()
            : ResolveRecoveryPosition();

        player_.SetPosition(recoveryPosition);
        player_.SetVelocity({ 0.0f, 0.0f });
        player_.SetBracing(false);
        stableGroundedSeconds_ = 0.0f;
        playerTerrainEffectResolver_.Reset();

        if (reason == PlayerRecoveryReason::Death)
        {
            player_.Revive(1.0f, 1.25f);
            ResetRecoveryState();
        }

        // 카메라도 즉시 붙여서, 복구 후 한 프레임 동안 빈 공간을 보지 않게 합니다.
        camera_.SnapTo(recoveryPosition);
    }

    Vector2 GameplayScene::ResolveRecoveryPosition() const
    {
        // 1순위: 플레이 중 직접 검증한 최근 안전 위치입니다.
        // 가장 플레이어 기대에 가까운 위치라서, 정적 SafePoint보다 우선합니다.
        if (lastSafePosition_.has_value())
        {
            return lastSafePosition_.value();
        }

        const WorldMap& world = level_.World();

        // 2순위: 맵 제작자가 LDtk에 찍어둔 SafePoint입니다.
        // 아직 안정 착지 기록이 없는 초반 낙하나 맵 로딩 직후 복구에 사용합니다.
        if (const std::optional<Vector2> safePoint = world.FirstSafePointPosition())
        {
            return safePoint.value();
        }

        // 3순위: 플레이어 시작 위치입니다.
        // SafePoint가 없는 테스트 맵에서도 최소한 시작점으로 돌아올 수 있게 합니다.
        if (const std::optional<Vector2> playerStart = world.PlayerStartPosition())
        {
            return playerStart.value();
        }

        // 최후 fallback입니다.
        // LDtk 데이터가 전혀 없거나 디버그 월드 생성 중 일부 값이 비어 있어도 게임이 멈추지 않게 합니다.
        return { 220.0f, level_.FloorWorldY() + 16.0f };
    }

    Vector2 GameplayScene::ResolveRespawnPosition() const
    {
        if (boundRespawnPosition_.has_value())
        {
            return boundRespawnPosition_.value();
        }

        const WorldMap& world = level_.World();
        if (const std::optional<Vector2> respawnPoint = world.FirstRespawnPointPosition())
        {
            return respawnPoint.value();
        }

        if (const std::optional<Vector2> playerStart = world.PlayerStartPosition())
        {
            return playerStart.value();
        }

        return { 220.0f, level_.FloorWorldY() + 16.0f };
    }

    void GameplayScene::DrawGameOverOverlay() const
    {
        if (playerLifeState_ != PlayerLifeState::GameOver)
        {
            return;
        }

        DrawRectangle(0, 0, static_cast<int>(camera_.ViewportSize().x), static_cast<int>(camera_.ViewportSize().y), Color{ 0, 0, 0, 135 });

        const char* title = "GAME OVER";
        const char* subtitle = "Reconstructing body at respawn anchor";
        const int titleFontSize = 48;
        const int subtitleFontSize = 22;
        const int titleWidth = MeasureText(title, titleFontSize);
        const int subtitleWidth = MeasureText(subtitle, subtitleFontSize);

        DrawText(
            title,
            (static_cast<int>(camera_.ViewportSize().x) - titleWidth) / 2,
            static_cast<int>(camera_.ViewportSize().y) / 2 - 46,
            titleFontSize,
            Color{ 235, 245, 255, 255 });
        DrawText(
            subtitle,
            (static_cast<int>(camera_.ViewportSize().x) - subtitleWidth) / 2,
            static_cast<int>(camera_.ViewportSize().y) / 2 + 18,
            subtitleFontSize,
            Color{ 170, 220, 230, 230 });
    }

    void GameplayScene::DrawInteractionPrompt() const
    {
        if (playerLifeState_ != PlayerLifeState::Alive
            || currentWorldInteraction_.candidate.type == WorldInteractionType::None
            || currentWorldInteraction_.activated
            || !currentWorldInteraction_.candidate.requiresInput)
        {
            return;
        }

        const char* action = "Use";
        if (currentWorldInteraction_.candidate.type == WorldInteractionType::Transition)
        {
            action = "Enter";
        }
        else if (currentWorldInteraction_.candidate.type == WorldInteractionType::RespawnPoint)
        {
            action = "Bind respawn";
        }

        const std::string text = std::string("E  ") + action;
        const int fontSize = 20;
        const int textWidth = MeasureText(text.c_str(), fontSize);
        const int paddingX = 18;
        const int paddingY = 10;
        const int boxWidth = textWidth + paddingX * 2;
        const int boxHeight = fontSize + paddingY * 2;
        const int x = (static_cast<int>(camera_.ViewportSize().x) - boxWidth) / 2;
        const int y = static_cast<int>(camera_.ViewportSize().y) - 92;

        DrawRectangle(x, y, boxWidth, boxHeight, Color{ 8, 12, 18, 180 });
        DrawRectangleLinesEx(
            Rectangle{
                static_cast<float>(x),
                static_cast<float>(y),
                static_cast<float>(boxWidth),
                static_cast<float>(boxHeight)
            },
            1.0f,
            Color{ 150, 220, 245, 190 });
        DrawText(
            text.c_str(),
            x + paddingX,
            y + paddingY,
            fontSize,
            Color{ 225, 245, 255, 240 });
    }
}
