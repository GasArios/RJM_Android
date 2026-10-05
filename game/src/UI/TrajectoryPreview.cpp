// TrajectoryPreview.cpp
// - 현재 조준 방향의 반동 이동 예상 경로를 계산하고 그립니다.
// - 계산은 실제 RecoilMovementController를 재사용해 튜닝값 변경에 자동으로 따라가게 합니다.

#include "RecoilJumpMan/UI/TrajectoryPreview.h"

#include "RecoilJumpMan/Physics/CoordinateSpace.h"
#include "RecoilJumpMan/Physics/RecoilMovementController.h"
#include "RecoilJumpMan/Physics/TileCollisionResolver.h"
#include "RecoilJumpMan/Weapons/Gun.h"
#include "RecoilJumpMan/World/TileMap.h"

#include <algorithm>

namespace
{
    // LerpByte:
    // - Color의 r/g/b/a는 unsigned char, 즉 0~255 사이 정수입니다.
    // - 색을 부드럽게 섞으려면 float로 계산한 뒤 다시 unsigned char로 바꿔야 합니다.
    // - std::clamp로 범위를 자르는 이유는 부동소수점 계산 중 255.0001 같은 값이 나올 수 있기 때문입니다.
    unsigned char LerpByte(unsigned char from, unsigned char to, float t)
    {
        const float value = static_cast<float>(from)
            + (static_cast<float>(to) - static_cast<float>(from)) * t;
        return static_cast<unsigned char>(std::clamp(value, 0.0f, 255.0f));
    }
}

namespace rjm
{
    // Clear:
    // - 이전 프레임의 궤적 샘플을 모두 지웁니다.
    // - 궤적은 마우스 위치, 플레이어 속도, 현재 총 상태에 따라 매 프레임 달라질 수 있습니다.
    void TrajectoryPreviewPath::Clear()
    {
        samples_.clear();
    }

    // AddSample:
    // - 계산된 궤적 점 하나를 추가합니다.
    // - normalizedTime은 렌더러가 색/투명도를 정할 때 쓰므로 0~1 범위로 보정합니다.
    // - grounded는 지금은 평평한 바닥 기준이지만, 나중에 타일 충돌 결과를 그대로 넣을 수 있습니다.
    void TrajectoryPreviewPath::AddSample(Vector2 worldPosition, float normalizedTime, bool grounded)
    {
        samples_.push_back(TrajectoryPreviewSample{
            worldPosition,
            std::clamp(normalizedTime, 0.0f, 1.0f),
            grounded
        });
    }

    // Samples:
    // - 샘플 배열을 읽기 전용으로 반환합니다.
    // - 렌더러가 이 배열을 순서대로 읽어 선과 점을 그립니다.
    const std::vector<TrajectoryPreviewSample>& TrajectoryPreviewPath::Samples() const
    {
        return samples_;
    }

    // Empty:
    // - 샘플이 하나도 없으면 true입니다.
    // - focus가 Gameplay이 아니거나, 총이 없거나, 발사 불가능하면 Build가 빈 경로를 남깁니다.
    bool TrajectoryPreviewPath::Empty() const
    {
        return samples_.empty();
    }

    // SetTuning:
    // - 미리보기 전용 튜닝값을 통째로 교체합니다.
    // - 예를 들어 옵션 메뉴에서 "궤적 미리보기 길이"를 바꾸면 이 함수로 새 값을 넣을 수 있습니다.
    void TrajectoryPreviewSimulator::SetTuning(TrajectoryPreviewTuning tuning)
    {
        tuning_ = tuning;
    }

    // Tuning:
    // - 현재 튜닝값을 읽기 전용으로 반환합니다.
    // - 디버그 오버레이나 설정 UI에서 현재 값을 표시할 때 사용할 수 있습니다.
    const TrajectoryPreviewTuning& TrajectoryPreviewSimulator::Tuning() const
    {
        return tuning_;
    }

    // Build:
    // - 현재 플레이어 상태와 마우스 조준 방향으로 "한 발 쐈을 때"의 예상 이동 경로를 만듭니다.
    // - 중요한 점:
    //   실제 Player 객체를 움직이지 않고, position/velocity 지역 변수에 현재 값을 복사해서 가상으로 굴립니다.
    // - 또 다른 중요한 점:
    //   별도 포물선 공식을 쓰지 않고 movement.ApplyRecoilImpulse / ApplyContinuousForces를 그대로 호출합니다.
    //   그래서 RecoilMovementTuning 값이 바뀌어도 실제 이동과 미리보기가 같은 규칙을 따릅니다.
    void TrajectoryPreviewSimulator::Build(
        const TrajectoryPreviewRequest& request,
        const RecoilMovementController& movement,
        TrajectoryPreviewPath& outPath) const
    {
        // 매 프레임 새 경로를 계산하므로, 이전 경로는 먼저 비웁니다.
        // 아래 조건에서 return하더라도 빈 경로가 남아서 화면에 오래된 궤적이 남지 않습니다.
        outPath.Clear();

        // 입력 focus가 Gameplay이 아니면 미리보기를 만들지 않습니다.
        // 예: 인벤토리, 지도, 설정창에서는 마우스가 UI를 가리키므로 궤적 보조가 방해됩니다.
        // gun이 nullptr인 경우는 현재 선택된 총이 없다는 뜻입니다.
        if (!AllowsTrajectoryPreview(request.inputFocus) || !request.gun)
        {
            return;
        }

        // 조준 방향은 "플레이어 중심 -> 마우스 월드 위치"입니다.
        // playerPosition 자체가 중심 좌표이므로 별도의 오프셋을 더하지 않습니다.
        // Gun::PreviewShot 안에서 이 방향을 정규화하고, 반동은 반대 방향으로 계산됩니다.
        const Vector2 aimDirection = {
            request.aimWorldTarget.x - request.playerPosition.x,
            request.aimWorldTarget.y - request.playerPosition.y
        };

        // PreviewShot:
        // - 탄약/쿨타임을 실제로 변경하지 않고 ShotResult만 계산합니다.
        // - requireReadyToFire가 true면 탄이 없거나 재장전 중일 때 궤적도 표시하지 않습니다.
        const ShotResult shot = request.gun->PreviewShot(aimDirection, tuning_.requireReadyToFire);
        if (!shot.fired)
        {
            return;
        }

        // 잘못된 튜닝값이 들어와도 계산이 망가지지 않도록 안전 범위로 보정합니다.
        // previewSeconds가 0 이하이면 보여줄 미래 시간이 없으므로 아래에서 return합니다.
        const float previewSeconds = std::max(0.0f, tuning_.previewSeconds);

        // stepSeconds는 시뮬레이션 한 칸의 시간입니다.
        // 너무 작으면 샘플이 많아지고, 너무 크면 궤적이 거칠어집니다.
        // 0.001f 미만은 불필요하게 촘촘하거나 0 나눗셈류 문제를 만들 수 있어 막습니다.
        const float stepSeconds = std::max(0.001f, tuning_.simulationStepSeconds);

        // 최소 2개 샘플은 있어야 선을 그릴 수 있습니다.
        // maxSampleCount가 0이나 1로 잘못 들어와도 렌더러가 최소한의 경로를 받을 수 있게 합니다.
        const int maxSampleCount = std::max(2, tuning_.maxSampleCount);

        if (previewSeconds <= 0.0f)
        {
            return;
        }

        // 실제 플레이어의 현재 상태를 지역 변수로 복사합니다.
        // 이후의 시뮬레이션은 이 복사본만 변경하므로, 프리뷰 계산이 실제 게임 상태에 영향을 주지 않습니다.
        Vector2 position = request.playerPosition;
        Vector2 velocity = request.playerVelocity;
        bool grounded = request.playerGrounded;

        // 지금 한 발 쐈다고 가정하고, 복사한 velocity에 반동 impulse를 적용합니다.
        // 이 호출도 실제 플레이어 이동과 같은 RecoilMovementController 함수를 사용합니다.
        const Vector2 adjustedRecoil = movement.BuildAirborneHeightAdjustedRecoilImpulse(
            shot.recoil,
            position.y,
            request.playerAirborneStartY,
            grounded,
            request.gun->Definition().altitudeDampingResistance);

        if (request.playerBracing)
        {
            movement.ApplyBracedRecoilImpulse(velocity, adjustedRecoil);
        }
        else
        {
            movement.ApplyRecoilImpulse(velocity, adjustedRecoil);
        }

        // 첫 샘플은 발사 직전/직후 기준 중심 위치입니다.
        // 시뮬레이션 위치와 표시 위치가 모두 중심 좌표이므로 그대로 저장합니다.
        outPath.AddSample(position, 0.0f, grounded);

        float elapsedSeconds = 0.0f;

        // Player::Update와 같은 resolver를 사용합니다.
        // 미리보기 전용으로 새 position/velocity만 굴리므로 실제 플레이어 상태는 바뀌지 않습니다.
        TileCollisionResolver tileCollision;
        while (elapsedSeconds < previewSeconds
            && static_cast<int>(outPath.Samples().size()) < maxSampleCount)
        {
            // 마지막 루프에서는 남은 시간이 stepSeconds보다 작을 수 있습니다.
            // std::min을 쓰면 previewSeconds를 정확히 넘지 않고 끝낼 수 있습니다.
            const float deltaSeconds = std::min(stepSeconds, previewSeconds - elapsedSeconds);

            // 실제 플레이어가 매 프레임 받는 지속 물리와 같은 함수를 사용합니다.
            // 여기에는 중력, 상승 저항, 정점 체공, 수평 마찰 같은 튜닝이 모두 들어 있습니다.
            movement.ApplyContinuousForces(velocity, deltaSeconds, grounded);

            if (request.useTileCollision && request.tileMap)
            {
                // 타일맵이 있으면 실제 지형 충돌을 반영합니다.
                // 이 경로 덕분에 궤적 선이 발판 위에서 멈추거나 벽에 막히는 모습을 미리 보여줄 수 있습니다.
                const TileCollisionMoveResult collision = tileCollision.MoveBox(
                    position,
                    velocity,
                    request.playerHalfSize,
                    deltaSeconds,
                    *request.tileMap);

                position = collision.position;
                velocity = collision.velocity;
                grounded = collision.grounded;
            }
            else
            {
                // Entity 이동과 같은 방식으로 속도 * 시간만큼 가상 위치를 갱신합니다.
                position.x += velocity.x * deltaSeconds;
                position.y += velocity.y * deltaSeconds;

                // 타일맵이 없을 때만 기존 평면 바닥 방식을 사용합니다.
                // LDtk 맵 로딩에 실패해도 디버그/프로토타입 미리보기가 완전히 사라지지 않게 하는 fallback입니다.
                if (tuning_.clampToGroundPlane
                    && request.useGroundPlane
                    && position.y - request.playerHalfSize <= request.groundY)
                {
                    position.y = request.groundY + request.playerHalfSize;
                    if (velocity.y < 0.0f)
                    {
                        velocity.y = 0.0f;
                    }
                    grounded = true;
                }
                else
                {
                    grounded = false;
                }
            }

            elapsedSeconds += deltaSeconds;

            // normalizedTime:
            // - 0이면 시작점, 1이면 previewSeconds 끝입니다.
            // - 렌더러가 이 값을 사용해 뒤로 갈수록 궤적을 흐리게 만듭니다.
            outPath.AddSample(position, elapsedSeconds / previewSeconds, grounded);
        }
    }

    // SetStyle:
    // - 현재 primitive 렌더러의 선/점 색과 크기를 교체합니다.
    // - 나중에 옵션이나 디버그 메뉴에서 궤적 표시 스타일을 바꿀 때 사용할 수 있습니다.
    void TrajectoryPreviewRenderer::SetStyle(TrajectoryPreviewRenderStyle style)
    {
        style_ = style;
    }

    // Style:
    // - 현재 렌더 스타일을 읽기 전용으로 반환합니다.
    const TrajectoryPreviewRenderStyle& TrajectoryPreviewRenderer::Style() const
    {
        return style_;
    }

    // Draw:
    // - TrajectoryPreviewPath에 저장된 월드 좌표 샘플을 Raylib 렌더 좌표로 바꿔 그립니다.
    // - 이 함수는 GameplayScene에서 camera_.Begin()과 camera_.End() 사이에 호출됩니다.
    // - 그래서 여기서는 화면 좌표가 아니라 "카메라 안의 렌더 좌표"까지만 변환하면 됩니다.
    void TrajectoryPreviewRenderer::Draw(const TrajectoryPreviewPath& path) const
    {
        const auto& samples = path.Samples();
        if (samples.size() < 2)
        {
            // 점이 0개 또는 1개면 선을 이을 수 없고, 현재 스타일상 의미 있는 궤적도 아닙니다.
            return;
        }

        for (std::size_t i = 1; i < samples.size(); ++i)
        {
            // 내부 월드 좌표는 y-up이고, Raylib 렌더 좌표는 y-down입니다.
            // CoordinateSpace가 그 차이를 처리합니다.
            const Vector2 previous = CoordinateSpace::WorldToRender(samples[i - 1].worldPosition);
            const Vector2 current = CoordinateSpace::WorldToRender(samples[i].worldPosition);
            const Color color = ColorForSample(samples[i]);

            // 선:
            // - 샘플 사이를 이어서 전체 이동 흐름을 보여줍니다.
            // - 나중에 에셋 기반으로 바꾸면 이 부분이 스프라이트 타일/잔상 렌더링으로 대체될 수 있습니다.
            if (style_.drawLines)
            {
                DrawLineEx(previous, current, style_.lineThickness, color);
            }

            // 점:
            // - 샘플 위치를 직접 표시해서 "시간 간격"을 읽기 쉽게 합니다.
            // - 점 간격이 넓으면 빠르게 이동하고, 촘촘하면 느리게 이동한다는 느낌을 줄 수 있습니다.
            if (style_.drawDots)
            {
                DrawCircleV(current, style_.dotRadius, color);
            }
        }
    }

    // LerpColor:
    // - 색 from과 to를 t 비율로 섞습니다.
    // - 궤적이 뒤로 갈수록 흐려지는 효과를 만들 때 사용합니다.
    Color TrajectoryPreviewRenderer::LerpColor(Color from, Color to, float t)
    {
        const float clampedT = std::clamp(t, 0.0f, 1.0f);
        return Color{
            LerpByte(from.r, to.r, clampedT),
            LerpByte(from.g, to.g, clampedT),
            LerpByte(from.b, to.b, clampedT),
            LerpByte(from.a, to.a, clampedT)
        };
    }

    // ColorForSample:
    // - 샘플 하나의 최종 색을 계산합니다.
    // - 기본적으로 normalizedTime이 커질수록 endColor에 가까워져 더 투명해집니다.
    // - grounded 샘플은 groundedColor를 살짝 섞어 착지/바닥 접촉 느낌을 줍니다.
    Color TrajectoryPreviewRenderer::ColorForSample(const TrajectoryPreviewSample& sample) const
    {
        const Color baseColor = LerpColor(style_.startColor, style_.endColor, sample.normalizedTime);
        if (!sample.grounded)
        {
            return baseColor;
        }

        return LerpColor(baseColor, style_.groundedColor, 0.45f);
    }
}

