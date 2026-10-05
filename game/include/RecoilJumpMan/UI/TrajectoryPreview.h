#pragma once

// TrajectoryPreview.h
// - 현재 마우스 조준 방향으로 한 발을 쐈을 때 플레이어가 움직일 예상 경로를 계산하고 그립니다.
// - 계산은 별도 점프 공식을 쓰지 않고 RecoilMovementController를 재사용합니다.
// - 렌더러는 지금 Raylib primitive를 쓰지만, 나중에 에셋 기반 렌더러로 교체할 수 있도록 분리했습니다.

#include "RecoilJumpMan/Core/InputFocus.h"

#include <raylib.h>

#include <vector>

namespace rjm
{
    // 전방 선언:
    // - 이 헤더에서는 Gun과 RecoilMovementController의 포인터/참조만 사용합니다.
    // - 전체 정의를 include하지 않아도 되므로, 빌드 의존성과 컴파일 시간을 줄일 수 있습니다.
    class Gun;
    class RecoilMovementController;
    class TileMap;

    // TrajectoryPreviewTuning:
    // - 궤적 미리보기의 "길이, 정밀도, 표시 조건"을 조절하는 값 묶음입니다.
    // - 실제 반동 이동 물리 수치는 RecoilMovementTuning에 있고,
    //   이 구조체는 미리보기 UI가 얼마나 많이/어떻게 샘플링할지만 정합니다.
    struct TrajectoryPreviewTuning
    {
        // 이 시간만큼 미래를 미리 봅니다. 실제 표시는 상황에 따라 더 짧을 수 있습니다.
        // 예: 1.0f면 지금 한 발 쏜 뒤 1초 동안의 예상 이동 경로를 계산합니다.
        float previewSeconds = 0.35f;

        // 시뮬레이션 샘플 간격입니다. 실제 물리 공식을 재사용하므로 너무 크게 두면 궤적이 거칠어집니다.
        // 1/60초는 실제 게임 프레임과 비슷한 간격이라 현재 프로토타입에 적당합니다.
        float simulationStepSeconds = 1.0f / 60.0f;

        // 데이터 실수로 너무 많은 점을 만들지 않기 위한 상한입니다.
        // previewSeconds가 커지거나 stepSeconds가 작아져도 무한히 많은 샘플을 만들지 않게 막습니다.
        int maxSampleCount = 90;

        // true면 현재 총이 실제로 발사 가능한 상태일 때만 미리보기를 보여줍니다.
        // 탄약이 없거나 재장전 중인 총의 궤적을 숨기면 "지금 당장 가능한 행동"만 보여줄 수 있습니다.
        bool requireReadyToFire = true;

        // 현재 프로토타입의 평평한 바닥을 시뮬레이션에 반영합니다.
        // 나중에 타일 충돌이 들어오면 이 경계를 더 풍부한 collision query로 바꿀 수 있습니다.
        bool clampToGroundPlane = true;
    };

    // TrajectoryPreviewSample:
    // - 궤적을 이루는 점 하나입니다.
    // - 이 구조체는 계산 결과이므로, 렌더러가 선/점/스프라이트/파티클 중 무엇으로 그릴지와 무관합니다.
    struct TrajectoryPreviewSample
    {
        // 샘플의 위치입니다.
        // Player와 Projectile처럼 y-up 월드 좌표계를 기준으로 저장합니다.
        // 기본 규칙상 중심 좌표입니다.
        Vector2 worldPosition = { 0.0f, 0.0f };

        // 전체 미리보기 시간 중 어느 지점인지 0~1로 표현합니다.
        // 렌더러는 이 값을 이용해 뒤쪽 샘플을 점점 흐리게 만들 수 있습니다.
        float normalizedTime = 0.0f;

        // 이 샘플 위치가 바닥에 닿아 있는지 나타냅니다.
        // 타일 충돌을 사용할 때는 TileCollisionResolver의 grounded 결과가 들어갑니다.
        bool grounded = false;
    };

    // TrajectoryPreviewPath:
    // - 여러 TrajectoryPreviewSample을 순서대로 담는 경로 데이터입니다.
    // - 계산 시스템은 이 객체를 채우고, 렌더링 시스템은 이 객체를 읽어서 화면에 표시합니다.
    // - 이 분리 덕분에 나중에 렌더링 방식을 바꿔도 계산 코드를 크게 건드리지 않아도 됩니다.
    class TrajectoryPreviewPath
    {
    public:
        // Clear:
        // - 이전 프레임에 계산한 샘플을 비웁니다.
        // - 궤적은 매 프레임 마우스 방향과 플레이어 속도에 따라 다시 계산됩니다.
        void Clear();

        // AddSample:
        // - 새 샘플 점을 경로 끝에 추가합니다.
        // - normalizedTime은 내부에서 0~1로 잘라서 잘못된 값이 렌더러로 넘어가지 않게 합니다.
        void AddSample(Vector2 worldPosition, float normalizedTime, bool grounded);

        // Samples:
        // - 경로에 들어 있는 샘플 목록을 읽기 전용 참조로 반환합니다.
        // - const 참조를 반환하므로 복사 비용 없이 읽을 수 있고, 외부에서 samples_를 직접 수정할 수 없습니다.
        const std::vector<TrajectoryPreviewSample>& Samples() const;

        // Empty:
        // - 표시할 샘플이 하나도 없는지 확인합니다.
        // - 현재는 렌더러가 samples.size()로도 판단하지만, 호출자 편의를 위해 둡니다.
        bool Empty() const;

    private:
        // 실제 샘플 배열입니다.
        // std::vector는 샘플 수가 튜닝값에 따라 달라질 수 있으므로 고정 배열보다 적합합니다.
        std::vector<TrajectoryPreviewSample> samples_;
    };

    // TrajectoryPreviewRequest:
    // - "이번 프레임에 궤적을 계산하기 위해 필요한 외부 상태"를 묶은 구조체입니다.
    // - Simulator가 Player, Camera, Level, InputState에 직접 의존하지 않게 하기 위해 사용합니다.
    // - 이렇게 해야 나중에 테스트 코드나 다른 씬에서도 같은 시뮬레이터를 재사용하기 쉽습니다.
    struct TrajectoryPreviewRequest
    {
        // 현재 입력 focus입니다.
        // Gameplay이 아니면 궤적 미리보기를 만들지 않습니다.
        InputFocus inputFocus = InputFocus::Gameplay;

        // 현재 선택된 총입니다.
        // nullptr이면 발사할 총이 없으므로 궤적을 만들지 않습니다.
        const Gun* gun = nullptr;

        // 현재 플레이어 중심 위치입니다.
        // 실제 플레이어를 움직이지 않고, 이 값을 복사한 가상 중심 위치만 시뮬레이션합니다.
        Vector2 playerPosition = { 0.0f, 0.0f };

        // 현재 플레이어 속도입니다.
        // 공중에서 이미 날아가고 있는 상태라면 그 속도까지 포함해 예상 경로를 계산해야 합니다.
        Vector2 playerVelocity = { 0.0f, 0.0f };

        // 현재 플레이어가 지상에 있는지입니다.
        // 실제 Player::Update와 같은 방식으로 첫 시뮬레이션 프레임의 마찰/공중 저항을 결정하는 데 사용합니다.
        bool playerGrounded = false;

        // 이번 체공을 시작했을 때의 플레이어 중심 y 좌표입니다.
        // 위쪽 반동 효율 감소가 실제 이동과 궤적 미리보기에서 같은 기준 높이를 쓰게 합니다.
        float playerAirborneStartY = 0.0f;

        // 현재 플레이어가 지상에서 버티기 중인지입니다.
        // true면 첫 발의 반동을 버티기 규칙으로 흡수해서 실제 이동과 예측선을 맞춥니다.
        bool playerBracing = false;

        // 마우스가 가리키는 월드 좌표입니다.
        // Simulator는 playerPosition -> aimWorldTarget 방향으로 총을 쏜다고 가정합니다.
        Vector2 aimWorldTarget = { 0.0f, 0.0f };

        // 플레이어 사각형 반 크기입니다.
        // playerPosition은 중심 좌표이므로, 바닥 접촉은 playerPosition.y - playerHalfSize로 판단합니다.
        float playerHalfSize = 0.0f;

        // 평평한 바닥 보정을 사용할지 여부입니다.
        // tileMap이 없거나 useTileCollision이 false인 경우에만 fallback으로 사용합니다.
        bool useGroundPlane = true;

        // 평평한 바닥의 월드 y 좌표입니다.
        // y-up 좌표계이므로 position.y - playerHalfSize가 이 값 이하가 되면 바닥에 닿은 것으로 봅니다.
        float groundY = 0.0f;

        // 지형 충돌에 사용할 타일맵입니다.
        // Player::Update와 같은 충돌 resolver에 넘겨 실제 이동과 미리보기를 맞춥니다.
        // nullptr이면 기존 평면 바닥 방식으로 미리보기를 계산합니다.
        const TileMap* tileMap = nullptr;

        // true면 tileMap 기반 충돌을 사용합니다.
        // tileMap이 nullptr이면 이 값이 true여도 fallback 바닥 경로로 내려갑니다.
        bool useTileCollision = false;
    };

    // TrajectoryPreviewSimulator:
    // - 현재 상태와 조준 방향을 받아 예상 경로를 계산합니다.
    // - 중요한 원칙:
    //   점프/낙하 공식을 새로 만들지 않고 RecoilMovementController를 그대로 호출합니다.
    // - 덕분에 반동 이동 튜닝값을 바꿔도 실제 이동과 미리보기 이동이 서로 어긋날 가능성이 줄어듭니다.
    class TrajectoryPreviewSimulator
    {
    public:
        // SetTuning:
        // - 미리보기 시간, 샘플 간격, 표시 조건 같은 UI 튜닝값을 교체합니다.
        void SetTuning(TrajectoryPreviewTuning tuning);

        // Tuning:
        // - 현재 미리보기 튜닝값을 읽기 전용으로 반환합니다.
        const TrajectoryPreviewTuning& Tuning() const;

        // Build:
        // - request에 담긴 현재 상태를 기반으로 outPath를 새로 계산합니다.
        // - movement는 실제 플레이어가 쓰는 RecoilMovementController입니다.
        // - outPath는 함수 시작 시 비워지므로, 실패 조건에서는 빈 경로가 됩니다.
        void Build(
            const TrajectoryPreviewRequest& request,
            const RecoilMovementController& movement,
            TrajectoryPreviewPath& outPath) const;

    private:
        // 미리보기 전용 튜닝값입니다.
        // 실제 점프 물리 수치가 아니라, "얼마나 오래/촘촘히 보여줄지"에 관한 값입니다.
        TrajectoryPreviewTuning tuning_;
    };

    // TrajectoryPreviewRenderStyle:
    // - 현재 Raylib primitive 렌더러가 사용할 시각 스타일입니다.
    // - 나중에 스프라이트 에셋 기반 렌더러로 교체해도 Path/Simulator는 그대로 둘 수 있습니다.
    struct TrajectoryPreviewRenderStyle
    {
        // true면 샘플 사이를 선으로 연결합니다.
        bool drawLines = true;

        // true면 각 샘플 위치에 점을 찍습니다.
        bool drawDots = true;

        // 궤적 선 두께입니다.
        float lineThickness = 2.0f;

        // 샘플 점 반지름입니다.
        float dotRadius = 3.0f;

        // 시작 지점에 가까운 샘플 색입니다.
        Color startColor = { 110, 205, 255, 170 };

        // 끝 지점에 가까운 샘플 색입니다.
        // alpha가 낮아서 뒤쪽으로 갈수록 자연스럽게 흐려집니다.
        Color endColor = { 110, 205, 255, 35 };

        // 바닥에 닿은 샘플에 섞을 색입니다.
        // 현재는 "착지 지점 느낌"을 조금 주기 위한 보조 색입니다.
        Color groundedColor = { 130, 230, 185, 120 };
    };

    // TrajectoryPreviewRenderer:
    // - TrajectoryPreviewPath를 화면에 그리는 클래스입니다.
    // - 지금은 DrawLineEx/DrawCircleV를 쓰는 간단한 렌더러입니다.
    // - 나중에 에셋을 쓰려면 이 클래스의 Draw 구현을 바꾸거나,
    //   같은 Path를 읽는 별도 SpriteTrajectoryPreviewRenderer를 만들 수 있습니다.
    class TrajectoryPreviewRenderer
    {
    public:
        // SetStyle:
        // - 현재 primitive 렌더러의 색, 두께, 점 크기 등을 교체합니다.
        void SetStyle(TrajectoryPreviewRenderStyle style);

        // Style:
        // - 현재 렌더 스타일을 읽기 전용으로 반환합니다.
        const TrajectoryPreviewRenderStyle& Style() const;

        // Draw:
        // - 경로 샘플들을 Raylib 렌더 좌표로 변환해 선/점으로 그립니다.
        // - 이 함수는 카메라 BeginMode2D 안에서 호출된다고 가정합니다.
        void Draw(const TrajectoryPreviewPath& path) const;

    private:
        // LerpColor:
        // - 두 색 사이를 t 비율로 섞습니다.
        // - t=0이면 from, t=1이면 to에 가깝습니다.
        static Color LerpColor(Color from, Color to, float t);

        // ColorForSample:
        // - 샘플의 normalizedTime과 grounded 상태를 이용해 최종 색을 결정합니다.
        Color ColorForSample(const TrajectoryPreviewSample& sample) const;

        // 현재 primitive 렌더링 스타일입니다.
        TrajectoryPreviewRenderStyle style_;
    };
}

