// SceneManager.cpp
// - SceneManager 클래스의 구현부입니다.
// - 현재 씬을 바꾸고, 매 프레임 현재 씬의 Update/Draw를 호출합니다.

#include "RecoilJumpMan/Scene/SceneManager.h"

#include "RecoilJumpMan/Scene/Scene.h"

namespace rjm
{
    // 기본 생성자 구현입니다.
    // = default는 컴파일러가 만들어주는 기본 동작을 쓰겠다는 뜻입니다.
    SceneManager::SceneManager() = default;

    // 소멸자 구현입니다.
    // 헤더가 아니라 .cpp에 두면 Scene의 전체 정의를 알고 있는 상태에서 unique_ptr이 삭제됩니다.
    SceneManager::~SceneManager() = default;

    // ChangeScene:
    // - std::move는 소유권을 옮길 때 쓰는 C++ 문법입니다.
    // - unique_ptr은 복사가 금지되어 있으므로, 새 씬을 current_로 옮길 때 std::move가 필요합니다.
    void SceneManager::ChangeScene(GameContext& context, std::unique_ptr<Scene> nextScene)
    {
        // 현재 씬이 있으면 먼저 종료 알림을 보냅니다.
        if (current_)
        {
            current_->OnExit(context);
        }

        // nextScene의 소유권을 current_로 이동시킵니다.
        current_ = std::move(nextScene);

        // 새 씬이 있으면 진입 알림을 보냅니다.
        if (current_)
        {
            current_->OnEnter(context);
        }
    }

    // Clear:
    // - 현재 씬을 종료하고 unique_ptr을 비웁니다.
    void SceneManager::Clear(GameContext& context)
    {
        if (current_)
        {
            current_->OnExit(context);

            // reset은 unique_ptr이 가진 객체를 삭제하고 nullptr 상태로 만듭니다.
            current_.reset();
        }
    }

    // Update:
    // - 현재 씬이 존재할 때만 Update를 호출합니다.
    void SceneManager::Update(GameContext& context, float deltaSeconds)
    {
        if (current_)
        {
            current_->Update(context, deltaSeconds);
        }
    }

    // Draw:
    // - 현재 씬이 존재할 때만 Draw를 호출합니다.
    void SceneManager::Draw(GameContext& context) const
    {
        if (current_)
        {
            current_->Draw(context);
        }
    }
}

