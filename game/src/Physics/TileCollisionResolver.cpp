// TileCollisionResolver.cpp
// - y-up 월드 좌표계에서 AABB와 solid 타일 충돌을 처리합니다.

#include "RecoilJumpMan/Physics/TileCollisionResolver.h"

#include "RecoilJumpMan/World/TileMap.h"

#include <algorithm>
#include <vector>

namespace rjm
{
    // MoveBox:
    // - velocity * deltaSeconds만큼 사각형을 움직이되, solid 타일과 겹치면 밖으로 밀어냅니다.
    // - x축과 y축을 한 번에 처리하지 않고 나눠 처리하면,
    //   대각선 이동 중 벽에 걸렸을 때 어느 축 때문에 막혔는지 더 단순하게 판단할 수 있습니다.
    TileCollisionMoveResult TileCollisionResolver::MoveBox(
        Vector2 position,
        Vector2 velocity,
        float halfSize,
        float deltaSeconds,
        const TileMap& tileMap) const
    {
        TileCollisionMoveResult result;
        result.position = position;
        result.velocity = velocity;

        if (deltaSeconds < 0.0f)
        {
            deltaSeconds = 0.0f;
        }

        std::vector<Rectangle> solidTiles;

        // 1단계: x축 이동과 벽 충돌 처리입니다.
        // y좌표는 아직 바꾸지 않고 좌우 벽에만 먼저 반응합니다.
        result.position.x += result.velocity.x * deltaSeconds;
        tileMap.CollectSolidTiles(BoxAt(result.position, halfSize), solidTiles);
        for (const Rectangle& solid : solidTiles)
        {
            if (!Intersects(BoxAt(result.position, halfSize), solid))
            {
                continue;
            }

            if (result.velocity.x > 0.0f)
            {
                result.position.x = solid.x - halfSize;
                result.hitWall = true;
            }
            else if (result.velocity.x < 0.0f)
            {
                result.position.x = solid.x + solid.width + halfSize;
                result.hitWall = true;
            }

            if (result.hitWall)
            {
                result.velocity.x = 0.0f;
            }
        }

        // 2단계: y축 이동과 바닥/천장 충돌 처리입니다.
        // y-up 좌표계이므로 velocity.y < 0은 아래로 떨어지는 중이고,
        // velocity.y > 0은 위로 올라가는 중입니다.
        result.position.y += result.velocity.y * deltaSeconds;
        tileMap.CollectSolidTiles(BoxAt(result.position, halfSize), solidTiles);
        for (const Rectangle& solid : solidTiles)
        {
            if (!Intersects(BoxAt(result.position, halfSize), solid))
            {
                continue;
            }

            if (result.velocity.y < 0.0f)
            {
                result.position.y = solid.y + solid.height + halfSize;
                result.velocity.y = 0.0f;
                result.grounded = true;
            }
            else if (result.velocity.y > 0.0f)
            {
                result.position.y = solid.y - halfSize;
                result.velocity.y = 0.0f;
                result.hitCeiling = true;
            }
        }

        // 3단계: 아주 가까운 바닥에 살짝 떠 있는 경우 착지로 보정합니다.
        // 반동 이동에서는 착지와 재장전 리듬이 중요하므로,
        // 몇 픽셀 차이로 착지가 실패하는 피로를 줄여줍니다.
        if (!result.grounded && result.velocity.y <= 0.0f)
        {
            TrySnapDown(result.position, result.velocity, halfSize, tileMap, result);
        }

        return result;
    }

    // BoxAt:
    // - Player::Position() 규칙과 맞춰 center는 사각형 중심입니다.
    Rectangle TileCollisionResolver::BoxAt(Vector2 center, float halfSize)
    {
        return {
            center.x - halfSize,
            center.y - halfSize,
            halfSize * 2.0f,
            halfSize * 2.0f
        };
    }

    // Intersects:
    // - Raylib의 CheckCollisionRecs는 화면 좌표 기준 느낌이 강하므로,
    //   여기서는 y-up 월드 Rectangle에도 그대로 맞는 AABB 겹침 공식을 직접 씁니다.
    bool TileCollisionResolver::Intersects(Rectangle a, Rectangle b)
    {
        return a.x < b.x + b.width
            && a.x + a.width > b.x
            && a.y < b.y + b.height
            && a.y + a.height > b.y;
    }

    // HorizontalOverlap:
    // - SnapDown은 "발밑에 바닥이 있는가"만 봐야 하므로 x축 겹침만 확인합니다.
    bool TileCollisionResolver::HorizontalOverlap(Rectangle a, Rectangle b)
    {
        return a.x < b.x + b.width
            && a.x + a.width > b.x;
    }

    // TrySnapDown:
    // - 현재 박스를 아래로 snapDownDistance_만큼 늘린 probe 영역을 만들고,
    //   그 안에 닿을 수 있는 solid 타일의 윗면을 찾습니다.
    void TileCollisionResolver::TrySnapDown(
        Vector2& position,
        Vector2& velocity,
        float halfSize,
        const TileMap& tileMap,
        TileCollisionMoveResult& result) const
    {
        Rectangle probe = BoxAt(position, halfSize);
        probe.y -= snapDownDistance_;
        probe.height += snapDownDistance_;

        std::vector<Rectangle> solidTiles;
        tileMap.CollectSolidTiles(probe, solidTiles);

        const Rectangle currentBox = BoxAt(position, halfSize);
        const float bottom = position.y - halfSize;
        float bestTop = -1.0f;

        // 여러 바닥 후보가 있으면 가장 높은 윗면을 고릅니다.
        // 그래야 얇은 모서리나 겹친 타일이 있을 때 더 자연스럽게 바로 아래 바닥에 붙습니다.
        for (const Rectangle& solid : solidTiles)
        {
            if (!HorizontalOverlap(currentBox, solid))
            {
                continue;
            }

            const float solidTop = solid.y + solid.height;
            const float distance = bottom - solidTop;
            if (distance < 0.0f || distance > snapDownDistance_)
            {
                continue;
            }

            bestTop = std::max(bestTop, solidTop);
        }

        if (bestTop >= 0.0f)
        {
            // 중심 좌표를 "바닥 윗면 + 반 크기"로 맞추면 사각형 아랫면이 바닥에 정확히 닿습니다.
            position.y = bestTop + halfSize;
            if (velocity.y < 0.0f)
            {
                velocity.y = 0.0f;
            }

            result.grounded = true;
            result.position = position;
            result.velocity = velocity;
        }
    }
}

