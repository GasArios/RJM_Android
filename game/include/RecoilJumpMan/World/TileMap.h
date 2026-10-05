#pragma once

// TileMap.h
// - 타일 기반 맵의 런타임 데이터를 표현합니다.
// - LDtk 같은 제작용 포맷은 로더에서만 알고, 게임플레이 코드는 이 TileMap을 기준으로 충돌을 질의합니다.

#include "RecoilJumpMan/Data/DefinitionId.h"

#include <raylib.h>

#include <cstddef>
#include <vector>

namespace rjm
{
    using TileCollisionMask = unsigned int;
    using TileEffectMask = unsigned int;

    enum TileCollisionFlags : TileCollisionMask
    {
        // 충돌 없음입니다.
        TileCollisionNone = 0,

        // 완전 고체 지형입니다.
        // Player와 궤적 미리보기는 현재 이 플래그만 실제 충돌 대상으로 처리합니다.
        TileCollisionSolid = 1 << 0,

        // 한 방향 발판 예약 플래그입니다.
        // 아직 물리 처리에는 쓰지 않지만, LDtk 값과 디버그 색을 미리 분리해 둡니다.
        TileCollisionOneWay = 1 << 1,
    };

    enum TileEffectFlags : TileEffectMask
    {
        TileEffectNone = 0,

        // 닿으면 일반 피해를 입는 타일입니다. 가시, 톱날, 뜨거운 표면 등에 사용합니다.
        TileEffectHazard = 1 << 0,

        // 겹쳐 있는 동안 일정 주기로 화염 피해를 주는 비고체 볼륨입니다.
        TileEffectLava = 1 << 1,

        // 겹치면 중력/저항을 바꾸는 물 볼륨입니다.
        TileEffectWater = 1 << 2,

        // 체력 피해가 아니라 마지막 안전 지점 복구를 요청하는 타일입니다.
        TileEffectRecovery = 1 << 3,

        // 표면 효과입니다. 플레이어 발밑 probe로 감지합니다.
        TileEffectIce = 1 << 4,
        TileEffectSticky = 1 << 5,
        TileEffectReloadSurface = 1 << 6,
        TileEffectConveyorLeft = 1 << 7,
        TileEffectConveyorRight = 1 << 8,

        // 볼륨 효과입니다. 바람/상승기류 같은 외부 가속도를 줍니다.
        TileEffectWindUp = 1 << 9,
        TileEffectWindLeft = 1 << 10,
        TileEffectWindRight = 1 << 11
    };

    struct TileEffectSample
    {
        int tileX = 0;
        int tileY = 0;
        Rectangle rect = { 0.0f, 0.0f, 0.0f, 0.0f };
        TileEffectMask effectFlags = TileEffectNone;
        DefinitionId effectId;
    };

    // TileCell:
    // - 타일 하나가 가진 런타임 정보입니다.
    // - visualId, collisionFlags, effectFlags를 분리해 보기/막힘/접촉 효과를 따로 다룰 수 있게 합니다.
    struct TileCell
    {
        // 렌더링용 타일 id입니다.
        // 0은 아직 그릴 타일이 없다는 뜻으로 사용합니다.
        int visualId = 0;

        // 막히는 지형/발판 같은 충돌 플래그입니다.
        TileCollisionMask collisionFlags = TileCollisionNone;

        // 닿았을 때 발생하는 효과 플래그입니다.
        TileEffectMask effectFlags = TileEffectNone;

        // 같은 플래그라도 세부 튜닝이 다른 경우를 위한 id입니다.
        DefinitionId effectId;
    };

    class TileMap
    {
    public:
        // Resize:
        // - 타일맵의 가로/세로 타일 수와 타일 크기를 설정합니다.
        void Resize(int width, int height, int tileSize);

        // DrawDebugGrid:
        // - 현재 타일맵 영역에 격자선을 그립니다.
        // - 개발 중 맵 크기를 확인하는 용도입니다.
        void DrawDebugGrid() const;

        // DrawDebugCollision:
        // - 충돌 타일을 반투명 사각형으로 그립니다.
        // - LDtk에서 읽힌 충돌 데이터가 게임 좌표로 맞게 들어왔는지 확인하는 용도입니다.
        void DrawDebugCollision() const;

        // SetVisualId:
        // - 특정 타일의 렌더링용 id를 저장합니다.
        // - 현재는 디버그 렌더링 중심이지만, 이후 타일셋 렌더러가 이 값을 사용할 수 있습니다.
        void SetVisualId(int tileX, int tileY, int visualId);

        // SetCollisionFlags:
        // - 특정 타일의 충돌 플래그를 설정합니다.
        void SetCollisionFlags(int tileX, int tileY, TileCollisionMask collisionFlags);

        // CollisionFlags:
        // - 특정 타일의 충돌 플래그를 반환합니다.
        TileCollisionMask CollisionFlags(int tileX, int tileY) const;

        // SetEffectFlags:
        // - 특정 타일의 접촉 효과 플래그를 설정합니다.
        void SetEffectFlags(int tileX, int tileY, TileEffectMask effectFlags, DefinitionId effectId = {});

        // EffectFlags:
        // - 특정 타일의 접촉 효과 플래그를 반환합니다.
        TileEffectMask EffectFlags(int tileX, int tileY) const;

        // EffectId:
        // - 특정 타일의 세부 효과 id를 반환합니다.
        DefinitionId EffectId(int tileX, int tileY) const;

        // IsSolidTile:
        // - 특정 타일이 완전 고체 지형인지 확인합니다.
        bool IsSolidTile(int tileX, int tileY) const;

        // IsSolidAtWorld:
        // - 월드 좌표 하나가 고체 타일 위에 있는지 확인합니다.
        bool IsSolidAtWorld(Vector2 worldPosition) const;

        // TileWorldRect:
        // - 타일 좌표를 y-up 월드 좌표 사각형으로 변환합니다.
        // - Rectangle의 x/y는 왼쪽 아래 기준으로 취급합니다.
        Rectangle TileWorldRect(int tileX, int tileY) const;

        // CollectSolidTiles:
        // - worldAabb와 겹칠 수 있는 solid 타일들의 월드 사각형을 반환합니다.
        void CollectSolidTiles(Rectangle worldAabb, std::vector<Rectangle>& outRects) const;

        // CollectEffectTiles:
        // - worldAabb와 겹치는 효과 타일 중 requiredEffects를 하나라도 가진 타일을 반환합니다.
        void CollectEffectTiles(
            Rectangle worldAabb,
            TileEffectMask requiredEffects,
            std::vector<TileEffectSample>& outTiles) const;

        // OverlapsEffect:
        // - worldAabb가 특정 효과 타일과 하나라도 겹치는지 빠르게 확인합니다.
        bool OverlapsEffect(Rectangle worldAabb, TileEffectMask requiredEffects) const;

        // Width:
        // - 타일맵의 가로 타일 개수를 반환합니다.
        int Width() const;

        // Height:
        // - 타일맵의 세로 타일 개수를 반환합니다.
        int Height() const;

        // TileSize:
        // - 타일 하나의 픽셀 크기를 반환합니다.
        int TileSize() const;

        // InBounds:
        // - 타일 좌표가 맵 내부인지 확인합니다.
        bool InBounds(int tileX, int tileY) const;

    private:
        // Index:
        // - 2D 타일 좌표를 cells_의 1D 배열 인덱스로 바꿉니다.
        // - 호출 전에 InBounds로 범위를 확인해야 합니다.
        std::size_t Index(int tileX, int tileY) const;

        // 가로 타일 개수입니다.
        int width_ = 0;

        // 세로 타일 개수입니다.
        int height_ = 0;

        // 타일 하나의 픽셀 크기입니다.
        int tileSize_ = 32;

        // 런타임 타일 셀 목록입니다.
        // tileY는 y-up 월드 기준입니다. 즉 tileY=0은 맵의 맨 아래 줄입니다.
        std::vector<TileCell> cells_;
    };
}

