// TileMap.cpp
// - TileMap 클래스의 구현부입니다.
// - LDtk에서 읽은 충돌/타일 데이터를 게임 런타임 좌표계로 보관합니다.

#include "RecoilJumpMan/World/TileMap.h"

#include "RecoilJumpMan/Physics/CoordinateSpace.h"

#include <algorithm>
#include <cmath>
#include <utility>

namespace rjm
{
    // Resize:
    // - 맵 크기와 타일 크기를 저장하고, 타일 배열을 새 크기에 맞게 채웁니다.
    void TileMap::Resize(int width, int height, int tileSize)
    {
        width_ = std::max(1, width);
        height_ = std::max(1, height);
        tileSize_ = std::max(1, tileSize);

        // assign(count, value):
        // - vector 크기를 count로 맞추고 모든 값을 value로 채웁니다.
        cells_.assign(width_ * height_, TileCell{});
    }

    // DrawDebugGrid:
    // - 세로선과 가로선을 그려 타일 격자를 표시합니다.
    void TileMap::DrawDebugGrid() const
    {
        const Color lineColor = Color{ 48, 52, 64, 255 };

        // 세로 격자선입니다.
        for (int x = 0; x <= width_; ++x)
        {
            DrawLineV(
                CoordinateSpace::WorldToRender({ static_cast<float>(x * tileSize_), 0.0f }),
                CoordinateSpace::WorldToRender({ static_cast<float>(x * tileSize_), static_cast<float>(height_ * tileSize_) }),
                lineColor);
        }

        // 가로 격자선입니다.
        for (int y = 0; y <= height_; ++y)
        {
            DrawLineV(
                CoordinateSpace::WorldToRender({ 0.0f, static_cast<float>(y * tileSize_) }),
                CoordinateSpace::WorldToRender({ static_cast<float>(width_ * tileSize_), static_cast<float>(y * tileSize_) }),
                lineColor);
        }
    }

    void TileMap::DrawDebugCollision() const
    {
        // 충돌 정보는 아직 실제 타일셋 렌더링과 별도로 그립니다.
        // 그래서 LDtk IntGrid 값이 제대로 뒤집혀 들어왔는지 눈으로 바로 확인할 수 있습니다.
        for (int y = 0; y < height_; ++y)
        {
            for (int x = 0; x < width_; ++x)
            {
                const TileCollisionMask collisionFlags = CollisionFlags(x, y);
                const TileEffectMask effectFlags = EffectFlags(x, y);
                if (collisionFlags == TileCollisionNone && effectFlags == TileEffectNone)
                {
                    continue;
                }

                Color fill = Color{ 80, 95, 120, 125 };
                if ((effectFlags & TileEffectLava) != 0)
                {
                    fill = Color{ 255, 96, 34, 135 };
                }
                else if ((effectFlags & TileEffectWater) != 0)
                {
                    fill = Color{ 50, 140, 230, 100 };
                }
                else if ((effectFlags & TileEffectHazard) != 0)
                {
                    fill = Color{ 210, 70, 70, 120 };
                }
                else if ((effectFlags & TileEffectRecovery) != 0)
                {
                    fill = Color{ 190, 80, 230, 120 };
                }
                else if ((effectFlags & TileEffectIce) != 0)
                {
                    fill = Color{ 130, 225, 255, 105 };
                }
                else if ((effectFlags & TileEffectSticky) != 0)
                {
                    fill = Color{ 120, 210, 100, 115 };
                }
                else if ((effectFlags & (TileEffectConveyorLeft | TileEffectConveyorRight)) != 0)
                {
                    fill = Color{ 245, 200, 75, 120 };
                }
                else if ((effectFlags & (TileEffectWindUp | TileEffectWindLeft | TileEffectWindRight)) != 0)
                {
                    fill = Color{ 150, 210, 255, 80 };
                }
                else if ((collisionFlags & TileCollisionOneWay) != 0)
                {
                    fill = Color{ 90, 190, 120, 110 };
                }

                const Rectangle worldRect = TileWorldRect(x, y);

                // DrawRectangleRec는 화면 y-down 좌표를 쓰므로,
                // y-up 월드 사각형의 왼쪽 위 꼭짓점을 렌더 좌표로 변환합니다.
                const Vector2 topLeft = CoordinateSpace::WorldToRender(
                    { worldRect.x, worldRect.y + worldRect.height });
                const Rectangle renderRect = {
                    topLeft.x,
                    topLeft.y,
                    worldRect.width,
                    worldRect.height
                };

                DrawRectangleRec(renderRect, fill);
            }
        }
    }

    void TileMap::SetVisualId(int tileX, int tileY, int visualId)
    {
        // 로더가 맵 바깥 타일을 실수로 넘겨도 조용히 무시합니다.
        // 제작 데이터가 조금 어긋나도 런타임이 터지지 않게 하는 방어입니다.
        if (!InBounds(tileX, tileY))
        {
            return;
        }

        cells_[Index(tileX, tileY)].visualId = visualId;
    }

    void TileMap::SetCollisionFlags(int tileX, int tileY, TileCollisionMask collisionFlags)
    {
        // LDtk IntGrid 크기와 TileMap 크기가 맞지 않는 경우를 대비한 범위 검사입니다.
        if (!InBounds(tileX, tileY))
        {
            return;
        }

        cells_[Index(tileX, tileY)].collisionFlags = collisionFlags;
    }

    TileCollisionMask TileMap::CollisionFlags(int tileX, int tileY) const
    {
        if (!InBounds(tileX, tileY))
        {
            return TileCollisionNone;
        }

        return cells_[Index(tileX, tileY)].collisionFlags;
    }

    void TileMap::SetEffectFlags(int tileX, int tileY, TileEffectMask effectFlags, DefinitionId effectId)
    {
        if (!InBounds(tileX, tileY))
        {
            return;
        }

        TileCell& cell = cells_[Index(tileX, tileY)];
        cell.effectFlags = effectFlags;
        cell.effectId = std::move(effectId);
    }

    TileEffectMask TileMap::EffectFlags(int tileX, int tileY) const
    {
        if (!InBounds(tileX, tileY))
        {
            return TileEffectNone;
        }

        return cells_[Index(tileX, tileY)].effectFlags;
    }

    DefinitionId TileMap::EffectId(int tileX, int tileY) const
    {
        if (!InBounds(tileX, tileY))
        {
            return {};
        }

        return cells_[Index(tileX, tileY)].effectId;
    }

    bool TileMap::IsSolidTile(int tileX, int tileY) const
    {
        return (CollisionFlags(tileX, tileY) & TileCollisionSolid) != 0;
    }

    bool TileMap::IsSolidAtWorld(Vector2 worldPosition) const
    {
        // 월드 좌표를 타일 좌표로 내림합니다.
        // y-up 좌표계에서 tileY=0은 맵의 가장 아래 줄입니다.
        const int tileX = static_cast<int>(std::floor(worldPosition.x / static_cast<float>(tileSize_)));
        const int tileY = static_cast<int>(std::floor(worldPosition.y / static_cast<float>(tileSize_)));
        return IsSolidTile(tileX, tileY);
    }

    Rectangle TileMap::TileWorldRect(int tileX, int tileY) const
    {
        // 이 Rectangle은 raylib 렌더용이 아니라 월드 충돌용입니다.
        // x/y를 왼쪽 아래로 해석하는 y-up 규칙을 코드 전체에서 맞춰 사용합니다.
        return {
            static_cast<float>(tileX * tileSize_),
            static_cast<float>(tileY * tileSize_),
            static_cast<float>(tileSize_),
            static_cast<float>(tileSize_)
        };
    }

    void TileMap::CollectSolidTiles(Rectangle worldAabb, std::vector<Rectangle>& outRects) const
    {
        outRects.clear();

        if (width_ <= 0 || height_ <= 0 || tileSize_ <= 0)
        {
            return;
        }

        // AABB의 오른쪽/위쪽 경계가 타일 경계에 정확히 닿았을 때,
        // 다음 타일까지 겹친 것으로 잘못 수집하지 않도록 아주 작은 값을 뺍니다.
        constexpr float edgeEpsilon = 0.001f;
        const int minTileX = std::max(
            0,
            static_cast<int>(std::floor(worldAabb.x / static_cast<float>(tileSize_))));
        const int maxTileX = std::min(
            width_ - 1,
            static_cast<int>(std::floor((worldAabb.x + worldAabb.width - edgeEpsilon) / static_cast<float>(tileSize_))));
        const int minTileY = std::max(
            0,
            static_cast<int>(std::floor(worldAabb.y / static_cast<float>(tileSize_))));
        const int maxTileY = std::min(
            height_ - 1,
            static_cast<int>(std::floor((worldAabb.y + worldAabb.height - edgeEpsilon) / static_cast<float>(tileSize_))));

        if (minTileX > maxTileX || minTileY > maxTileY)
        {
            return;
        }

        for (int tileY = minTileY; tileY <= maxTileY; ++tileY)
        {
            for (int tileX = minTileX; tileX <= maxTileX; ++tileX)
            {
                if (IsSolidTile(tileX, tileY))
                {
                    // resolver는 Rectangle 목록만 알면 되므로,
                    // TileMap 내부 인덱스 구조를 밖으로 노출하지 않습니다.
                    outRects.push_back(TileWorldRect(tileX, tileY));
                }
            }
        }
    }

    void TileMap::CollectEffectTiles(
        Rectangle worldAabb,
        TileEffectMask requiredEffects,
        std::vector<TileEffectSample>& outTiles) const
    {
        outTiles.clear();

        if (requiredEffects == TileEffectNone || width_ <= 0 || height_ <= 0 || tileSize_ <= 0)
        {
            return;
        }

        constexpr float edgeEpsilon = 0.001f;
        const int minTileX = std::max(
            0,
            static_cast<int>(std::floor(worldAabb.x / static_cast<float>(tileSize_))));
        const int maxTileX = std::min(
            width_ - 1,
            static_cast<int>(std::floor((worldAabb.x + worldAabb.width - edgeEpsilon) / static_cast<float>(tileSize_))));
        const int minTileY = std::max(
            0,
            static_cast<int>(std::floor(worldAabb.y / static_cast<float>(tileSize_))));
        const int maxTileY = std::min(
            height_ - 1,
            static_cast<int>(std::floor((worldAabb.y + worldAabb.height - edgeEpsilon) / static_cast<float>(tileSize_))));

        if (minTileX > maxTileX || minTileY > maxTileY)
        {
            return;
        }

        for (int tileY = minTileY; tileY <= maxTileY; ++tileY)
        {
            for (int tileX = minTileX; tileX <= maxTileX; ++tileX)
            {
                const TileEffectMask effectFlags = EffectFlags(tileX, tileY);
                if ((effectFlags & requiredEffects) == 0)
                {
                    continue;
                }

                outTiles.push_back(TileEffectSample{
                    tileX,
                    tileY,
                    TileWorldRect(tileX, tileY),
                    effectFlags,
                    EffectId(tileX, tileY)
                });
            }
        }
    }

    bool TileMap::OverlapsEffect(Rectangle worldAabb, TileEffectMask requiredEffects) const
    {
        std::vector<TileEffectSample> effectTiles;
        CollectEffectTiles(worldAabb, requiredEffects, effectTiles);
        return !effectTiles.empty();
    }

    // Width:
    // - 가로 타일 개수를 반환합니다.
    int TileMap::Width() const
    {
        return width_;
    }

    // Height:
    // - 세로 타일 개수를 반환합니다.
    int TileMap::Height() const
    {
        return height_;
    }

    // TileSize:
    // - 타일 하나의 픽셀 크기를 반환합니다.
    int TileMap::TileSize() const
    {
        return tileSize_;
    }

    bool TileMap::InBounds(int tileX, int tileY) const
    {
        return tileX >= 0
            && tileY >= 0
            && tileX < width_
            && tileY < height_;
    }

    std::size_t TileMap::Index(int tileX, int tileY) const
    {
        // cells_는 행 우선(row-major)으로 저장합니다.
        // tileY가 아래에서 위로 증가한다는 점만 화면 좌표계와 다릅니다.
        return static_cast<std::size_t>(tileY * width_ + tileX);
    }
}
