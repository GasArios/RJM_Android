#pragma once

// ViewportScaler.h
// - 실제 창 크기와 게임의 가상 해상도 사이를 연결하는 클래스입니다.
// - 플레이어는 창 크기를 자유롭게 바꿀 수 있지만, 게임 화면은 항상 16:9 비율을 유지합니다.
// - 남는 영역은 레터박스/필러박스 영역으로 남겨두고, 나중에 일러스트나 장식 UI를 그릴 수 있습니다.

#include <raylib.h>

namespace rjm
{
    class ViewportScaler
    {
    public:
        // 생성자:
        // - 게임이 내부적으로 그릴 가상 화면 크기를 받습니다.
        // - 예: 1280x720은 16:9 가상 화면입니다.
        ViewportScaler(int virtualWidth, int virtualHeight);

        // Update:
        // - 현재 실제 창 크기를 받아, 창 안에 들어갈 16:9 게임 화면 영역을 다시 계산합니다.
        // - 창 크기는 GetScreenWidth()/GetScreenHeight()로 매 프레임 달라질 수 있습니다.
        void Update(int windowWidth, int windowHeight);
        // Android uses the complete aspect ratio at a constant reference height.
        void UpdateFullWidth(int windowWidth, int windowHeight);

        // WindowToVirtual:
        // - 실제 창 좌표계를 가상 화면 좌표계로 변환합니다.
        // - 마우스 좌표를 카메라와 HUD가 쓰는 1280x720 기준 좌표로 바꿀 때 사용합니다.
        Vector2 WindowToVirtual(Vector2 windowPosition) const;

        // ContainsWindowPoint:
        // - 실제 창 좌표가 게임 화면 viewport 안에 있는지 확인합니다.
        // - 레터박스 영역 위에 마우스가 있을 때 조준/발사를 막는 데 사용합니다.
        bool ContainsWindowPoint(Vector2 windowPosition) const;

        // DrawLetterboxDecorations:
        // - 게임 화면 바깥의 남는 영역을 그립니다.
        // - 지금은 예시용 도형을 그리지만, 나중에는 이 함수에서 에셋 일러스트를 그릴 수 있습니다.
        void DrawLetterboxDecorations() const;

        // DrawGameTexture:
        // - 1280x720 가상 화면에 그려진 RenderTexture를 실제 창의 viewport 영역에 확대/축소해서 그립니다.
        void DrawGameTexture(const RenderTexture2D& target) const;

        // Viewport:
        // - 실제 창 안에서 게임 화면이 차지하는 사각형입니다.
        Rectangle Viewport() const;

        // Scale:
        // - 가상 화면 픽셀 하나가 실제 창에서 몇 픽셀로 보이는지 나타냅니다.
        float Scale() const;

        // VirtualWidth / VirtualHeight:
        // - 게임이 내부적으로 사용하는 가상 해상도입니다.
        int VirtualWidth() const;
        int VirtualHeight() const;

        // WindowWidth / WindowHeight:
        // - 현재 실제 창 크기입니다.
        int WindowWidth() const;
        int WindowHeight() const;

    private:
        int virtualWidth_ = 1280;
        int virtualHeight_ = 720;
        int windowWidth_ = 1280;
        int windowHeight_ = 720;
        float scale_ = 1.0f;
        Rectangle viewport_ = { 0.0f, 0.0f, 1280.0f, 720.0f };
    };
}

