#pragma once

// TileCollisionResolver.h
// - 사각형 캐릭터를 TileMap 위에서 움직이는 최소 지형 충돌 계산기입니다.
// - Player와 TrajectoryPreview가 같은 resolver를 쓰게 해서 실제 이동과 예측 궤적을 맞춥니다.

#include <raylib.h>

namespace rjm
{
    class TileMap;

    // TileCollisionMoveResult:
    // - 타일 충돌 계산이 끝난 뒤 플레이어/프리뷰에 돌려줄 결과입니다.
    // - 위치와 속도는 충돌 보정이 끝난 최종 값입니다.
    struct TileCollisionMoveResult
    {
        // 충돌 보정 후 중심 월드 좌표입니다.
        Vector2 position = { 0.0f, 0.0f };

        // 벽, 바닥, 천장에 막힌 뒤의 최종 속도입니다.
        // 예: 바닥에 착지하면 y속도는 0이 됩니다.
        Vector2 velocity = { 0.0f, 0.0f };

        // 아래 방향 이동 중 solid 타일 위에 착지했는지 나타냅니다.
        bool grounded = false;

        // 좌우 방향 이동 중 벽에 막혔는지 나타냅니다.
        bool hitWall = false;

        // 위 방향 이동 중 천장에 막혔는지 나타냅니다.
        bool hitCeiling = false;
    };

    // TileCollisionResolver:
    // - TileMap의 solid 타일과 사각형 캐릭터의 충돌을 처리합니다.
    // - 현재는 축 분리 방식으로 x축 이동 후 y축 이동을 처리하는 최소 구현입니다.
    // - Player와 TrajectoryPreview가 같은 MoveBox를 호출해 실제 이동과 궤적 예측이 어긋나지 않게 합니다.
    class TileCollisionResolver
    {
    public:
        // MoveBox:
        // - 현재 중심 위치와 속도, 반 크기, deltaSeconds를 받아 한 프레임 이동을 계산합니다.
        // - 반환값에는 벽/바닥/천장 충돌로 보정된 위치와 속도가 들어 있습니다.
        TileCollisionMoveResult MoveBox(
            Vector2 position,
            Vector2 velocity,
            float halfSize,
            float deltaSeconds,
            const TileMap& tileMap) const;

    private:
        // BoxAt:
        // - 중심 좌표와 반 크기로 AABB 사각형을 만듭니다.
        static Rectangle BoxAt(Vector2 center, float halfSize);

        // Intersects:
        // - y-up 월드 좌표계에서 두 Rectangle이 겹치는지 확인합니다.
        static bool Intersects(Rectangle a, Rectangle b);

        // HorizontalOverlap:
        // - 착지 보정 때 x축 범위만 겹치는지 확인합니다.
        static bool HorizontalOverlap(Rectangle a, Rectangle b);

        // TrySnapDown:
        // - 플레이어가 바닥 바로 위에 살짝 떠 있는 경우 아래로 붙여 착지 처리합니다.
        // - 반동 이동 게임에서 모서리/착지 감각이 너무 빡빡해지는 것을 줄이기 위한 보정입니다.
        void TrySnapDown(
            Vector2& position,
            Vector2& velocity,
            float halfSize,
            const TileMap& tileMap,
            TileCollisionMoveResult& result) const;

        // 발밑 이 거리 안에 solid 바닥이 있으면 착지로 보정합니다.
        float snapDownDistance_ = 6.0f;
    };
}

