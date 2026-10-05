#pragma once

// AssetManager.h
// - 이미지, 사운드, 폰트 같은 게임 리소스를 관리하기 위한 클래스 선언부입니다.
// - 현재는 Texture2D만 관리합니다.
// - Raylib 리소스는 직접 Unload해야 하므로, 한곳에서 관리하는 구조가 중요합니다.

#include <raylib.h>

#include <string>
#include <unordered_map>

namespace rjm
{
    class AssetManager
    {
    public:
        // 기본 생성자입니다.
        AssetManager() = default;

        // 소멸자:
        // - AssetManager 객체가 사라질 때 로드한 텍스처를 정리합니다.
        ~AssetManager();

        // 복사 생성자 삭제:
        // - Texture2D 같은 리소스 소유 객체가 복사되면 같은 리소스를 두 번 해제할 위험이 있습니다.
        // - 그래서 복사를 금지합니다.
        AssetManager(const AssetManager&) = delete;

        // 복사 대입 연산자 삭제:
        // - 이미 존재하는 AssetManager에 다른 AssetManager를 복사 대입하는 것도 금지합니다.
        AssetManager& operator=(const AssetManager&) = delete;

        // LoadTextureAsset:
        // - id라는 이름으로 path에 있는 텍스처를 로드합니다.
        // - 이미 같은 id가 있으면 새로 로드하지 않고 기존 텍스처를 반환합니다.
        Texture2D& LoadTextureAsset(const std::string& id, const std::string& path);

        // FindTexture:
        // - id로 텍스처를 찾습니다.
        // - 찾으면 포인터를 반환하고, 없으면 nullptr를 반환합니다.
        Texture2D* FindTexture(const std::string& id);

        // const 버전 FindTexture:
        // - const AssetManager에서도 텍스처를 찾을 수 있게 하는 함수입니다.
        // - 반환된 Texture2D도 const 포인터라 수정할 수 없습니다.
        const Texture2D* FindTexture(const std::string& id) const;

        // UnloadAll:
        // - 로드된 모든 텍스처를 Raylib에 반납하고 목록을 비웁니다.
        void UnloadAll();

    private:
        // unordered_map:
        // - key와 value를 연결해서 저장하는 해시 테이블입니다.
        // - 여기서는 문자열 id로 Texture2D를 찾습니다.
        std::unordered_map<std::string, Texture2D> textures_;
    };
}

