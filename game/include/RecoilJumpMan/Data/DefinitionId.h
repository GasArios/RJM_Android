#pragma once

// DefinitionId.h
// - 데이터 정의를 식별하기 위한 id 타입을 한곳에 정의합니다.
// - 예: "old_revolver", "forest_boar", "starter_field" 같은 문자열 id입니다.

#include <string>

namespace rjm
{
    // using:
    // - 타입 별칭을 만드는 C++ 문법입니다.
    // - DefinitionId는 실제로 std::string과 같습니다.
    // - 하지만 의미상 "데이터 정의의 id"라는 뜻이 더 잘 드러납니다.
    using DefinitionId = std::string;
}

