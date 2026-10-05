#pragma once

// CoordinateSpace.h
// - 게임 내부 물리 좌표계와 Raylib 카메라가 그리는 렌더 좌표계를 분리하는 도우미입니다.
//
// Recoil Jump Man 물리 좌표:
// - y가 커질수록 위로 올라갑니다.
// - 반동 점프 수식, 중력, 낙하 속도는 이 좌표계를 기준으로 계산합니다.
//
// Raylib 렌더 좌표:
// - y가 커질수록 아래로 내려갑니다.
// - BeginMode2D(Camera2D) 안에서 실제로 그릴 때 사용하는 좌표입니다.
//
// Screen 좌표:
// - 실제 창 픽셀 좌표입니다.
// - Screen <-> World 변환은 카메라 상태가 필요하므로 GameCamera가 담당합니다.

#include <raylib.h>

namespace rjm
{
    class CoordinateSpace
    {
    public:
        // WorldToRender:
        // - 게임 내부 월드 좌표를 Raylib 렌더 좌표로 변환합니다.
        static Vector2 WorldToRender(Vector2 worldPosition);

        // RenderToWorld:
        // - Raylib 렌더 좌표를 게임 내부 월드 좌표로 변환합니다.
        static Vector2 RenderToWorld(Vector2 renderPosition);

        // WorldDirectionToRender:
        // - 위치가 아니라 방향 벡터를 변환합니다.
        static Vector2 WorldDirectionToRender(Vector2 worldDirection);

        // RenderDirectionToWorld:
        // - Raylib 렌더 방향 벡터를 게임 내부 물리 방향으로 변환합니다.
        static Vector2 RenderDirectionToWorld(Vector2 renderDirection);
    };
}

