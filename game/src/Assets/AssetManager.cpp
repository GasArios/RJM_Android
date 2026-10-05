// AssetManager.cpp
// - AssetManager 클래스의 구현부입니다.
// - Raylib 텍스처를 로드하고, 찾고, 해제하는 로직이 있습니다.

#include "RecoilJumpMan/Assets/AssetManager.h"

namespace rjm
{
    // 소멸자:
    // - 객체가 사라질 때 자동 호출됩니다.
    // - 리소스 누수를 막기 위해 UnloadAll을 호출합니다.
    AssetManager::~AssetManager()
    {
        UnloadAll();
    }

    // LoadTextureAsset:
    // - id로 이미 로드된 텍스처가 있는지 먼저 검사합니다.
    Texture2D& AssetManager::LoadTextureAsset(const std::string& id, const std::string& path)
    {
        // find는 unordered_map에서 key를 찾는 함수입니다.
        // 찾지 못하면 textures_.end()를 반환합니다.
        auto found = textures_.find(id);
        if (found != textures_.end())
        {
            // found->second는 map에 저장된 value, 즉 Texture2D입니다.
            return found->second;
        }

        // Raylib의 LoadTexture 함수로 파일에서 텍스처를 로드합니다.
        Texture2D texture = ::LoadTexture(path.c_str());

        // emplace는 map에 새 key-value 쌍을 직접 생성해 넣습니다.
        // 반환값의 first는 삽입된 원소를 가리키는 iterator입니다.
        auto inserted = textures_.emplace(id, texture);
        return inserted.first->second;
    }

    // FindTexture:
    // - id로 텍스처를 찾아 수정 가능한 포인터를 반환합니다.
    Texture2D* AssetManager::FindTexture(const std::string& id)
    {
        auto found = textures_.find(id);

        // 삼항 연산자:
        // - 조건 ? 참일 때 값 : 거짓일 때 값
        return found == textures_.end() ? nullptr : &found->second;
    }

    // const 버전 FindTexture:
    // - const 객체에서 호출되므로 반환 포인터도 const Texture2D*입니다.
    const Texture2D* AssetManager::FindTexture(const std::string& id) const
    {
        auto found = textures_.find(id);
        return found == textures_.end() ? nullptr : &found->second;
    }

    // UnloadAll:
    // - map에 저장된 모든 Texture2D를 Raylib의 UnloadTexture로 해제합니다.
    void AssetManager::UnloadAll()
    {
        // 범위 기반 for문입니다.
        // textures_ 안의 모든 key-value 쌍을 순회합니다.
        for (auto& entry : textures_)
        {
            UnloadTexture(entry.second);
        }

        // clear는 map 안의 모든 원소를 제거합니다.
        textures_.clear();
    }
}

