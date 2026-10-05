// ViewportScaler.cpp
// - 실제 창 크기 안에 16:9 가상 게임 화면을 비율 유지로 배치합니다.
// - 남는 공간은 검은 바 또는 장식 영역으로 사용할 수 있습니다.

#include "RecoilJumpMan/Core/ViewportScaler.h"

#include <algorithm>
#include <cmath>

namespace
{
    // DrawDecoratedBar:
    // - 레터박스/필러박스 영역에 임시 장식 도형을 그립니다.
    // - 나중에 일러스트 에셋이 준비되면 이 함수의 내용을 이미지 렌더링으로 교체할 수 있습니다.
    void DrawDecoratedBar(Rectangle rect)
    {
        if (rect.width <= 0.0f || rect.height <= 0.0f)
        {
            return;
        }

        DrawRectangleRec(rect, Color{ 10, 12, 18, 255 });
        DrawRectangleLinesEx(rect, 2.0f, Color{ 40, 56, 78, 210 });

        constexpr float spacing = 36.0f;
        for (float x = rect.x - rect.height; x < rect.x + rect.width + rect.height; x += spacing)
        {
            DrawLineEx(
                { x, rect.y + rect.height },
                { x + rect.height, rect.y },
                1.0f,
                Color{ 28, 36, 52, 140 });
        }
    }
}

namespace rjm
{
    ViewportScaler::ViewportScaler(int virtualWidth, int virtualHeight)
        : virtualWidth_(std::max(1, virtualWidth)),
          virtualHeight_(std::max(1, virtualHeight))
    {
        Update(virtualWidth_, virtualHeight_);
    }

    void ViewportScaler::Update(int windowWidth, int windowHeight)
    {
        windowWidth_ = std::max(1, windowWidth);
        windowHeight_ = std::max(1, windowHeight);

        const float scaleX = static_cast<float>(windowWidth_) / static_cast<float>(virtualWidth_);
        const float scaleY = static_cast<float>(windowHeight_) / static_cast<float>(virtualHeight_);
        scale_ = std::min(scaleX, scaleY);

        const float viewportWidth = static_cast<float>(virtualWidth_) * scale_;
        const float viewportHeight = static_cast<float>(virtualHeight_) * scale_;

        viewport_ = {
            (static_cast<float>(windowWidth_) - viewportWidth) * 0.5f,
            (static_cast<float>(windowHeight_) - viewportHeight) * 0.5f,
            viewportWidth,
            viewportHeight
        };
    }

    void ViewportScaler::UpdateFullWidth(int windowWidth, int windowHeight)
    {
        const int safeWidth = std::max(1, windowWidth);
        const int safeHeight = std::max(1, windowHeight);
        virtualWidth_ = std::max(1, static_cast<int>(std::round(
            static_cast<double>(virtualHeight_) * safeWidth / safeHeight)));
        Update(safeWidth, safeHeight);
    }

    Vector2 ViewportScaler::WindowToVirtual(Vector2 windowPosition) const
    {
        return {
            (windowPosition.x - viewport_.x) / scale_,
            (windowPosition.y - viewport_.y) / scale_
        };
    }

    bool ViewportScaler::ContainsWindowPoint(Vector2 windowPosition) const
    {
        return windowPosition.x >= viewport_.x
            && windowPosition.y >= viewport_.y
            && windowPosition.x <= viewport_.x + viewport_.width
            && windowPosition.y <= viewport_.y + viewport_.height;
    }

    void ViewportScaler::DrawLetterboxDecorations() const
    {
        DrawRectangle(0, 0, windowWidth_, windowHeight_, Color{ 6, 7, 10, 255 });

        const Rectangle top = {
            0.0f,
            0.0f,
            static_cast<float>(windowWidth_),
            viewport_.y
        };
        const Rectangle bottom = {
            0.0f,
            viewport_.y + viewport_.height,
            static_cast<float>(windowWidth_),
            static_cast<float>(windowHeight_) - (viewport_.y + viewport_.height)
        };
        const Rectangle left = {
            0.0f,
            viewport_.y,
            viewport_.x,
            viewport_.height
        };
        const Rectangle right = {
            viewport_.x + viewport_.width,
            viewport_.y,
            static_cast<float>(windowWidth_) - (viewport_.x + viewport_.width),
            viewport_.height
        };

        DrawDecoratedBar(top);
        DrawDecoratedBar(bottom);
        DrawDecoratedBar(left);
        DrawDecoratedBar(right);
    }

    void ViewportScaler::DrawGameTexture(const RenderTexture2D& target) const
    {
        const Rectangle source = {
            0.0f,
            0.0f,
            static_cast<float>(virtualWidth_),
            -static_cast<float>(virtualHeight_)
        };

        DrawTexturePro(target.texture, source, viewport_, { 0.0f, 0.0f }, 0.0f, WHITE);
    }

    Rectangle ViewportScaler::Viewport() const
    {
        return viewport_;
    }

    float ViewportScaler::Scale() const
    {
        return scale_;
    }

    int ViewportScaler::VirtualWidth() const
    {
        return virtualWidth_;
    }

    int ViewportScaler::VirtualHeight() const
    {
        return virtualHeight_;
    }

    int ViewportScaler::WindowWidth() const
    {
        return windowWidth_;
    }

    int ViewportScaler::WindowHeight() const
    {
        return windowHeight_;
    }
}

