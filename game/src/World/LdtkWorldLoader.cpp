// LdtkWorldLoader.cpp
// - LDtk JSON에서 런타임 충돌, 구역, 전환, 스폰, 안전 위치 데이터를 읽습니다.
// - 외부 JSON 라이브러리 없이 쓰는 최소 로더라서, LDtk 포맷 의존 코드는 이 파일 안에 모아둡니다.

#include "RecoilJumpMan/World/LdtkWorldLoader.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <utility>

namespace
{
    // JsonValue:
    // - LDtk 파일을 읽기 위한 아주 작은 JSON 값 표현입니다.
    // - 현재 로더가 필요한 타입만 담고, 게임 코드 밖으로 노출하지 않습니다.
    struct JsonValue
    {
        enum class Type
        {
            Null,
            Bool,
            Number,
            String,
            Array,
            Object
        };

        Type type = Type::Null;
        bool boolValue = false;
        double numberValue = 0.0;
        std::string stringValue;
        std::vector<JsonValue> arrayValue;
        std::unordered_map<std::string, JsonValue> objectValue;
    };

    // JsonParser:
    // - 외부 JSON 라이브러리를 추가하지 않고 LDtk 파일을 읽기 위한 최소 파서입니다.
    // - 범용 JSON 기능을 완벽히 제공하는 목적이 아니라, LDtk가 내보내는 표준 JSON을 읽는 데 집중합니다.
    class JsonParser
    {
    public:
        explicit JsonParser(const std::string& text)
            : text_(text)
        {
        }

        // Parse:
        // - 문자열 전체를 JsonValue 트리로 바꿉니다.
        // - 끝에 알 수 없는 문자가 남으면 실패 처리해 깨진 파일을 조용히 받아들이지 않습니다.
        bool Parse(JsonValue& outValue, std::string& outError)
        {
            SkipWhitespace();
            if (!ParseValue(outValue, outError))
            {
                return false;
            }

            SkipWhitespace();
            if (position_ != text_.size())
            {
                outError = "Unexpected trailing JSON data.";
                return false;
            }

            return true;
        }

    private:
        // ParseValue:
        // - 현재 토큰의 첫 글자를 보고 object/array/string/number/literal 중 하나로 분기합니다.
        bool ParseValue(JsonValue& outValue, std::string& outError)
        {
            SkipWhitespace();
            if (position_ >= text_.size())
            {
                outError = "Unexpected end of JSON.";
                return false;
            }

            const char c = text_[position_];
            if (c == '{')
            {
                return ParseObject(outValue, outError);
            }
            if (c == '[')
            {
                return ParseArray(outValue, outError);
            }
            if (c == '"')
            {
                std::string value;
                if (!ParseString(value, outError))
                {
                    return false;
                }

                outValue.type = JsonValue::Type::String;
                outValue.stringValue = std::move(value);
                return true;
            }
            if (c == '-' || std::isdigit(static_cast<unsigned char>(c)) != 0)
            {
                return ParseNumber(outValue, outError);
            }
            if (MatchLiteral("true"))
            {
                outValue.type = JsonValue::Type::Bool;
                outValue.boolValue = true;
                return true;
            }
            if (MatchLiteral("false"))
            {
                outValue.type = JsonValue::Type::Bool;
                outValue.boolValue = false;
                return true;
            }
            if (MatchLiteral("null"))
            {
                outValue.type = JsonValue::Type::Null;
                return true;
            }

            outError = "Unexpected JSON token.";
            return false;
        }

        // ParseObject:
        // - { "key": value } 형태를 unordered_map으로 읽습니다.
        bool ParseObject(JsonValue& outValue, std::string& outError)
        {
            ++position_;
            outValue.type = JsonValue::Type::Object;
            outValue.objectValue.clear();

            SkipWhitespace();
            if (TryConsume('}'))
            {
                return true;
            }

            while (position_ < text_.size())
            {
                std::string key;
                if (!ParseString(key, outError))
                {
                    return false;
                }

                SkipWhitespace();
                if (!TryConsume(':'))
                {
                    outError = "Expected ':' after JSON object key.";
                    return false;
                }

                JsonValue value;
                if (!ParseValue(value, outError))
                {
                    return false;
                }

                outValue.objectValue[std::move(key)] = std::move(value);

                SkipWhitespace();
                if (TryConsume('}'))
                {
                    return true;
                }
                if (!TryConsume(','))
                {
                    outError = "Expected ',' or '}' in JSON object.";
                    return false;
                }
                SkipWhitespace();
            }

            outError = "Unterminated JSON object.";
            return false;
        }

        // ParseArray:
        // - [value, value] 형태를 순서가 있는 vector로 읽습니다.
        bool ParseArray(JsonValue& outValue, std::string& outError)
        {
            ++position_;
            outValue.type = JsonValue::Type::Array;
            outValue.arrayValue.clear();

            SkipWhitespace();
            if (TryConsume(']'))
            {
                return true;
            }

            while (position_ < text_.size())
            {
                JsonValue value;
                if (!ParseValue(value, outError))
                {
                    return false;
                }

                outValue.arrayValue.push_back(std::move(value));

                SkipWhitespace();
                if (TryConsume(']'))
                {
                    return true;
                }
                if (!TryConsume(','))
                {
                    outError = "Expected ',' or ']' in JSON array.";
                    return false;
                }
                SkipWhitespace();
            }

            outError = "Unterminated JSON array.";
            return false;
        }

        // ParseString:
        // - JSON 문자열 escape를 최소한으로 처리합니다.
        // - unicode escape는 현재 런타임 id/레이어명에 필요하지 않아 '?'로 보관합니다.
        bool ParseString(std::string& outString, std::string& outError)
        {
            if (!TryConsume('"'))
            {
                outError = "Expected JSON string.";
                return false;
            }

            outString.clear();
            while (position_ < text_.size())
            {
                const char c = text_[position_++];
                if (c == '"')
                {
                    return true;
                }

                if (c != '\\')
                {
                    outString.push_back(c);
                    continue;
                }

                if (position_ >= text_.size())
                {
                    outError = "Unterminated JSON string escape.";
                    return false;
                }

                const char escaped = text_[position_++];
                switch (escaped)
                {
                case '"': outString.push_back('"'); break;
                case '\\': outString.push_back('\\'); break;
                case '/': outString.push_back('/'); break;
                case 'b': outString.push_back('\b'); break;
                case 'f': outString.push_back('\f'); break;
                case 'n': outString.push_back('\n'); break;
                case 'r': outString.push_back('\r'); break;
                case 't': outString.push_back('\t'); break;
                case 'u':
                    if (position_ + 4 > text_.size())
                    {
                        outError = "Invalid unicode escape in JSON string.";
                        return false;
                    }
                    position_ += 4;
                    outString.push_back('?');
                    break;
                default:
                    outError = "Invalid escape in JSON string.";
                    return false;
                }
            }

            outError = "Unterminated JSON string.";
            return false;
        }

        // ParseNumber:
        // - strtod로 JSON 숫자를 double로 읽고, 필요한 곳에서 int/float로 변환합니다.
        bool ParseNumber(JsonValue& outValue, std::string& outError)
        {
            const char* begin = text_.c_str() + position_;
            char* end = nullptr;
            const double value = std::strtod(begin, &end);
            if (begin == end)
            {
                outError = "Invalid JSON number.";
                return false;
            }

            position_ += static_cast<std::size_t>(end - begin);
            outValue.type = JsonValue::Type::Number;
            outValue.numberValue = value;
            return true;
        }

        // MatchLiteral:
        // - true/false/null 같은 고정 토큰을 소비합니다.
        bool MatchLiteral(const char* literal)
        {
            const std::size_t length = std::char_traits<char>::length(literal);
            if (text_.compare(position_, length, literal) != 0)
            {
                return false;
            }

            position_ += length;
            return true;
        }

        // TryConsume:
        // - 현재 문자가 기대한 문자와 같으면 한 글자 소비합니다.
        bool TryConsume(char c)
        {
            if (position_ >= text_.size() || text_[position_] != c)
            {
                return false;
            }

            ++position_;
            return true;
        }

        // SkipWhitespace:
        // - JSON 문법상 의미 없는 공백을 건너뜁니다.
        void SkipWhitespace()
        {
            while (position_ < text_.size()
                && std::isspace(static_cast<unsigned char>(text_[position_])) != 0)
            {
                ++position_;
            }
        }

        const std::string& text_;
        std::size_t position_ = 0;
    };

    // ReadTextFile:
    // - LDtk JSON 파일 전체를 문자열로 읽습니다.
    // - 실패 이유의 상세 메시지는 호출자가 path와 함께 만들어 반환합니다.
    bool ReadTextFile(const std::string& path, std::string& outText)
    {
#ifdef __ANDROID__
        // Android APK assets are not ordinary filesystem paths.
        const std::string assetPath = path.rfind("assets/", 0) == 0 ? path.substr(7) : path;
        char* text = LoadFileText(assetPath.c_str());
        if (!text) return false;
        outText = text;
        UnloadFileText(text);
        return true;
#else
        std::ifstream file(path);
        if (!file)
        {
            return false;
        }

        std::ostringstream stream;
        stream << file.rdbuf();
        outText = stream.str();
        return true;
#endif
    }

    // Member:
    // - JSON object에서 이름이 맞는 멤버를 찾아 읽기 전용 포인터로 돌려줍니다.
    // - 타입이 object가 아니거나 멤버가 없으면 nullptr입니다.
    const JsonValue* Member(const JsonValue& value, const std::string& name)
    {
        if (value.type != JsonValue::Type::Object)
        {
            return nullptr;
        }

        const auto found = value.objectValue.find(name);
        return found == value.objectValue.end() ? nullptr : &found->second;
    }

    // ArrayMember:
    // - object 멤버 중 배열 타입만 편하게 꺼내기 위한 헬퍼입니다.
    const std::vector<JsonValue>* ArrayMember(const JsonValue& value, const std::string& name)
    {
        const JsonValue* member = Member(value, name);
        if (!member || member->type != JsonValue::Type::Array)
        {
            return nullptr;
        }

        return &member->arrayValue;
    }

    // StringMember:
    // - object 멤버가 문자열이면 반환하고, 없거나 타입이 다르면 fallback을 반환합니다.
    std::string StringMember(const JsonValue& value, const std::string& name, const std::string& fallback = {})
    {
        const JsonValue* member = Member(value, name);
        if (!member || member->type != JsonValue::Type::String)
        {
            return fallback;
        }

        return member->stringValue;
    }

    // IntMember / FloatMember:
    // - LDtk 숫자 필드를 런타임에서 쓰는 정수/실수 타입으로 변환합니다.
    int IntMember(const JsonValue& value, const std::string& name, int fallback = 0)
    {
        const JsonValue* member = Member(value, name);
        if (!member || member->type != JsonValue::Type::Number)
        {
            return fallback;
        }

        return static_cast<int>(member->numberValue);
    }

    float FloatMember(const JsonValue& value, const std::string& name, float fallback = 0.0f)
    {
        const JsonValue* member = Member(value, name);
        if (!member || member->type != JsonValue::Type::Number)
        {
            return fallback;
        }

        return static_cast<float>(member->numberValue);
    }

    // ReadFloatArray2:
    // - LDtk의 px처럼 [x, y] 형태인 숫자 배열을 읽습니다.
    bool ReadFloatArray2(const JsonValue& value, const std::string& name, float& outX, float& outY)
    {
        const std::vector<JsonValue>* array = ArrayMember(value, name);
        if (!array || array->size() < 2
            || (*array)[0].type != JsonValue::Type::Number
            || (*array)[1].type != JsonValue::Type::Number)
        {
            return false;
        }

        outX = static_cast<float>((*array)[0].numberValue);
        outY = static_cast<float>((*array)[1].numberValue);
        return true;
    }

    // FieldValue:
    // - LDtk entity의 fieldInstances 배열에서 __identifier가 name인 필드의 __value를 찾습니다.
    const JsonValue* FieldValue(const JsonValue& entity, const std::string& name)
    {
        const std::vector<JsonValue>* fields = ArrayMember(entity, "fieldInstances");
        if (!fields)
        {
            return nullptr;
        }

        for (const JsonValue& field : *fields)
        {
            if (StringMember(field, "__identifier") == name)
            {
                return Member(field, "__value");
            }
        }

        return nullptr;
    }

    // FieldString / FieldFloat:
    // - entity custom field를 런타임 타입으로 꺼내는 헬퍼입니다.
    std::string FieldString(const JsonValue& entity, const std::string& name, const std::string& fallback = {})
    {
        const JsonValue* value = FieldValue(entity, name);
        if (!value || value->type != JsonValue::Type::String)
        {
            return fallback;
        }

        return value->stringValue;
    }

    // FieldStringAny:
    // - 같은 의미의 필드명을 여러 후보로 허용합니다.
    // - LDtk 스키마 이름을 바꾸는 실험 중에도 기존 맵을 조금 더 잘 읽기 위한 장치입니다.
    std::string FieldStringAny(
        const JsonValue& entity,
        const std::vector<std::string>& names,
        const std::string& fallback = {})
    {
        for (const std::string& name : names)
        {
            const std::string value = FieldString(entity, name);
            if (!value.empty())
            {
                return value;
            }
        }

        return fallback;
    }

    float FieldFloat(const JsonValue& entity, const std::string& name, float fallback = 0.0f)
    {
        const JsonValue* value = FieldValue(entity, name);
        if (!value || value->type != JsonValue::Type::Number)
        {
            return fallback;
        }

        return static_cast<float>(value->numberValue);
    }

    // ParseTraversalTier:
    // - LDtk 문자열 필드를 런타임 TraversalTier enum으로 변환합니다.
    rjm::TraversalTier ParseTraversalTier(const std::string& value)
    {
        if (value == "T1") return rjm::TraversalTier::T1;
        if (value == "T2") return rjm::TraversalTier::T2;
        if (value == "T3") return rjm::TraversalTier::T3;
        if (value == "T4") return rjm::TraversalTier::T4;
        if (value == "T5") return rjm::TraversalTier::T5;
        return rjm::TraversalTier::T0;
    }

    // ParseTransitionKind:
    // - Transition 엔티티의 kind 문자열을 터치/상호작용 트리거로 변환합니다.
    rjm::TransitionKind ParseTransitionKind(const std::string& value)
    {
        return value == "Interact" || value == "interact"
            ? rjm::TransitionKind::Interact
            : rjm::TransitionKind::Touch;
    }

    rjm::RespawnPointKind ParseRespawnPointKind(const std::string& value)
    {
        if (value == "Town" || value == "town") return rjm::RespawnPointKind::Town;
        if (value == "Camp" || value == "camp") return rjm::RespawnPointKind::Camp;
        if (value == "BossGate" || value == "boss_gate") return rjm::RespawnPointKind::BossGate;
        if (value == "Debug" || value == "debug") return rjm::RespawnPointKind::Debug;
        return rjm::RespawnPointKind::FieldEntrance;
    }

    struct TileRuntimeFlags
    {
        rjm::TileCollisionMask collisionFlags = rjm::TileCollisionNone;
        rjm::TileEffectMask effectFlags = rjm::TileEffectNone;
        rjm::DefinitionId effectId;
    };

    TileRuntimeFlags RuntimeFlagsFromIntGridValue(int value)
    {
        // LDtk IntGrid 값 약속:
        // 0: 없음, 1: Solid, 2: OneWay, 3: Hazard, 4: Solid+Hazard,
        // 5: Lava, 6: Water, 7: Recovery, 8: Ice, 9: Sticky, 10: ReloadSurface,
        // 11: ConveyorLeft, 12: ConveyorRight, 13: WindUp, 14: WindLeft, 15: WindRight.
        // 새 충돌 타입을 추가하면 LDtk enum/IntGrid 값과 이 매핑을 같이 갱신해야 합니다.
        switch (value)
        {
        case 1:
            return { rjm::TileCollisionSolid, rjm::TileEffectNone, {} };
        case 2:
            return { rjm::TileCollisionOneWay, rjm::TileEffectNone, {} };
        case 3:
            return { rjm::TileCollisionNone, rjm::TileEffectHazard, "terrain_hazard" };
        case 4:
            return { rjm::TileCollisionSolid, rjm::TileEffectHazard, "terrain_hazard_solid" };
        case 5:
            return { rjm::TileCollisionNone, rjm::TileEffectLava, "terrain_lava" };
        case 6:
            return { rjm::TileCollisionNone, rjm::TileEffectWater, "terrain_water" };
        case 7:
            return { rjm::TileCollisionNone, rjm::TileEffectRecovery, "terrain_recovery" };
        case 8:
            return { rjm::TileCollisionSolid, rjm::TileEffectIce, "terrain_ice" };
        case 9:
            return { rjm::TileCollisionSolid, rjm::TileEffectSticky, "terrain_sticky" };
        case 10:
            return { rjm::TileCollisionSolid, rjm::TileEffectReloadSurface, "terrain_reload_surface" };
        case 11:
            return { rjm::TileCollisionSolid, rjm::TileEffectConveyorLeft, "terrain_conveyor_left" };
        case 12:
            return { rjm::TileCollisionSolid, rjm::TileEffectConveyorRight, "terrain_conveyor_right" };
        case 13:
            return { rjm::TileCollisionNone, rjm::TileEffectWindUp, "terrain_wind_up" };
        case 14:
            return { rjm::TileCollisionNone, rjm::TileEffectWindLeft, "terrain_wind_left" };
        case 15:
            return { rjm::TileCollisionNone, rjm::TileEffectWindRight, "terrain_wind_right" };
        default:
            return {};
        }
    }

    Rectangle EntityRectToWorld(const JsonValue& entity, float levelHeight)
    {
        float px = 0.0f;
        float py = 0.0f;
        ReadFloatArray2(entity, "px", px, py);

        const float width = FloatMember(entity, "width", 0.0f);
        const float height = FloatMember(entity, "height", 0.0f);

        // LDtk의 px는 왼쪽 위 기준 y-down 좌표입니다.
        // 게임 월드는 왼쪽 아래 기준 y-up 좌표라서 levelHeight - py - height로 뒤집습니다.
        return {
            px,
            levelHeight - py - height,
            width,
            height
        };
    }

    Vector2 RectCenter(Rectangle rect)
    {
        // 엔티티 위치는 Player/Enemy 규칙과 맞춰 중심 좌표로 저장합니다.
        return {
            rect.x + rect.width * 0.5f,
            rect.y + rect.height * 0.5f
        };
    }

    const JsonValue* SelectLevel(const JsonValue& root, const rjm::LdtkLoadOptions& options)
    {
        // .ldtkl 같은 단일 레벨 JSON은 root에 layerInstances가 바로 있습니다.
        // .ldtk 프로젝트 파일은 levels 배열 안에서 선택해야 합니다.
        if (Member(root, "layerInstances"))
        {
            return &root;
        }

        const std::vector<JsonValue>* levels = ArrayMember(root, "levels");
        if (!levels || levels->empty())
        {
            return nullptr;
        }

        if (options.levelIdentifier.empty())
        {
            // 아직 레벨 선택 UI가 없으므로 identifier를 지정하지 않으면 첫 레벨을 사용합니다.
            return &(*levels)[0];
        }

        for (const JsonValue& level : *levels)
        {
            if (StringMember(level, "identifier") == options.levelIdentifier)
            {
                return &level;
            }
        }

        return nullptr;
    }

    void ReadCollisionLayer(
        const JsonValue& layer,
        int levelWidth,
        int levelHeight,
        const rjm::LdtkLoadOptions& options,
        rjm::TileMap& tileMap)
    {
        const int gridSize = IntMember(layer, "__gridSize", options.defaultTileSize);
        const int cWid = IntMember(layer, "__cWid", std::max(1, levelWidth / gridSize));
        const int cHei = IntMember(layer, "__cHei", std::max(1, levelHeight / gridSize));

        tileMap.Resize(cWid, cHei, gridSize);

        const std::vector<JsonValue>* values = ArrayMember(layer, "intGridCsv");
        if (!values)
        {
            return;
        }

        // LDtk CSV는 위에서 아래로 저장됩니다.
        // TileMap은 y-up이므로 yDown을 뒤집어 tileY=0이 맵의 맨 아래 줄이 되게 합니다.
        for (int yDown = 0; yDown < cHei; ++yDown)
        {
            for (int x = 0; x < cWid; ++x)
            {
                const int index = yDown * cWid + x;
                if (index >= static_cast<int>(values->size()) || (*values)[index].type != JsonValue::Type::Number)
                {
                    continue;
                }

                const int tileY = cHei - 1 - yDown;
                const int value = static_cast<int>((*values)[index].numberValue);
                const TileRuntimeFlags flags = RuntimeFlagsFromIntGridValue(value);
                tileMap.SetCollisionFlags(x, tileY, flags.collisionFlags);
                tileMap.SetEffectFlags(x, tileY, flags.effectFlags, flags.effectId);
            }
        }
    }

    void ReadVisualTiles(const JsonValue& layer, rjm::TileMap& tileMap)
    {
        const std::vector<JsonValue>* tiles = ArrayMember(layer, "gridTiles");
        if (!tiles)
        {
            return;
        }

        const int gridSize = IntMember(layer, "__gridSize", tileMap.TileSize());
        const int cHei = IntMember(layer, "__cHei", tileMap.Height());

        for (const JsonValue& tile : *tiles)
        {
            float px = 0.0f;
            float py = 0.0f;
            if (!ReadFloatArray2(tile, "px", px, py))
            {
                continue;
            }

            const int tileX = static_cast<int>(px) / gridSize;

            // gridTiles의 py도 y-down이므로 IntGrid와 같은 방식으로 뒤집습니다.
            const int tileY = cHei - 1 - (static_cast<int>(py) / gridSize);
            const int visualId = IntMember(tile, "t", 0) + 1;
            tileMap.SetVisualId(tileX, tileY, visualId);
        }
    }

    void ReadEntity(
        const JsonValue& entity,
        float levelHeight,
        rjm::LdtkLoadedWorld& outWorld)
    {
        const std::string type = StringMember(entity, "__identifier");
        const Rectangle rect = EntityRectToWorld(entity, levelHeight);
        const std::string iid = StringMember(entity, "iid", type);

        if (type == "PlayerStart")
        {
            // PlayerStart는 시작 위치 전용이라 SafePoint 목록에는 넣지 않습니다.
            // 복구 순서에서는 SafePoint가 없을 때의 fallback으로 사용됩니다.
            outWorld.playerStartPosition = RectCenter(rect);
            return;
        }

        if (type == "SafePoint")
        {
            // SafePoint는 맵 이탈 복구용 정적 후보입니다.
            // safePointId 필드가 없으면 LDtk iid를 id로 사용해도 고유성을 유지할 수 있습니다.
            outWorld.safePoints.push_back(rjm::SafePoint{
                FieldString(entity, "safePointId", iid),
                RectCenter(rect),
                static_cast<int>(FieldFloat(entity, "priority", 0.0f))
            });
            return;
        }

        if (type == "RespawnPoint")
        {
            // RespawnPoint는 HP 0 사망 후 임시 육체를 재구성할 부활 앵커입니다.
            // SafePoint와 달리 마을, 캠프, 필드 입구, 보스문 앞처럼 논리적 복귀 지점에 찍습니다.
            outWorld.respawnPoints.push_back(rjm::RespawnPoint{
                FieldString(entity, "respawnPointId", iid),
                RectCenter(rect),
                ParseRespawnPointKind(FieldString(entity, "kind", "FieldEntrance")),
                static_cast<int>(FieldFloat(entity, "priority", 0.0f))
            });
            return;
        }

        if (type == "EnemySpawn")
        {
            // LDtk 엔티티 필드가 비어 있으면 디버그 적 기본값을 사용합니다.
            // 덕분에 맵에 spawn만 찍어도 최소 테스트가 가능합니다.
            const std::string spawnId = FieldString(entity, "spawnId", iid);
            const std::string enemyId = FieldString(entity, "enemyId", "debug_slime");
            outWorld.spawnPoints.push_back(rjm::SpawnPoint{
                spawnId,
                enemyId,
                RectCenter(rect),
                FieldFloat(entity, "activationRadius", 1400.0f),
                FieldFloat(entity, "deactivationRadius", 1800.0f),
                FieldFloat(entity, "respawnSeconds", 8.0f)
            });
            return;
        }

        if (type == "Transition")
        {
            // targetLevelId/targetMapId처럼 이름이 조금 달라져도 읽을 수 있게 여러 필드명을 허용합니다.
            // LDtk 스키마를 실험하는 동안 로더 수정 부담을 줄이기 위한 완충입니다.
            outWorld.transitions.push_back(rjm::TransitionTrigger{
                FieldString(entity, "transitionId", iid),
                rect,
                ParseTransitionKind(FieldString(entity, "kind", "Touch")),
                FieldStringAny(entity, { "targetLevelId", "targetMapId", "targetMap" }),
                FieldStringAny(entity, { "targetSpawnId", "targetSpawn" })
            });
            return;
        }

        if (type == "WorldAnchor" || type == "TransitionAnchor")
        {
            // Transition.targetSpawnId가 이 anchorId를 가리키면, GameplayScene이 해당 좌표로 플레이어를 옮깁니다.
            // 이름은 targetSpawnId로 남아 있지만 실제 의미는 "도착 앵커 id"에 가깝습니다.
            outWorld.anchors.push_back(rjm::WorldAnchor{
                FieldStringAny(entity, { "anchorId", "spawnId", "targetSpawnId" }, iid),
                RectCenter(rect)
            });
            return;
        }

        if (type == "AreaZone")
        {
            // AreaZone은 게임 안 지도/난이도 구역을 위한 논리적 영역입니다.
            // 현재는 visibleBounds와 bounds를 같은 값으로 두고, 나중에 지도 표시용 영역을 따로 분리할 수 있습니다.
            const std::string areaId = FieldString(entity, "areaId", iid);
            outWorld.areas.push_back(rjm::Area{
                areaId,
                FieldString(entity, "displayName", areaId),
                rect,
                rect,
                ParseTraversalTier(FieldString(entity, "traversalTier", "T0"))
            });
            return;
        }
    }

    // BuildWorldFromLevel:
    // - 선택된 LDtk level 하나를 Recoil Jump Man 런타임 월드 데이터로 변환합니다.
    // - 충돌 레이어를 먼저 읽어 TileMap 크기를 정하고, 이후 타일/엔티티 레이어를 채웁니다.
    bool BuildWorldFromLevel(
        const JsonValue& level,
        rjm::LdtkLoadedWorld& outWorld,
        std::string& error,
        const rjm::LdtkLoadOptions& options)
    {
        const int levelWidth = IntMember(level, "pxWid", 0);
        const int levelHeight = IntMember(level, "pxHei", 0);
        if (levelWidth <= 0 || levelHeight <= 0)
        {
            error = "LDtk level is missing pxWid/pxHei.";
            return false;
        }

        outWorld = rjm::LdtkLoadedWorld{};
        outWorld.worldBounds = {
            0.0f,
            0.0f,
            static_cast<float>(levelWidth),
            static_cast<float>(levelHeight)
        };

        const std::vector<JsonValue>* layers = ArrayMember(level, "layerInstances");
        if (!layers)
        {
            error = "LDtk level has no layerInstances. If this project uses external levels, load the .ldtkl file or keep level data embedded.";
            return false;
        }

        bool foundCollisionLayer = false;
        for (const JsonValue& layer : *layers)
        {
            const std::string identifier = StringMember(layer, "__identifier");
            if (identifier == options.collisionLayerName)
            {
                ReadCollisionLayer(layer, levelWidth, levelHeight, options, outWorld.tileMap);
                foundCollisionLayer = true;
                break;
            }
        }

        if (!foundCollisionLayer)
        {
            // 충돌 레이어가 없어도 빈 TileMap은 만들어둡니다.
            // 이렇게 해야 카메라/월드 경계/디버그 그리기 코드가 null 타일맵을 따로 처리하지 않아도 됩니다.
            const int tileSize = std::max(1, options.defaultTileSize);
            outWorld.tileMap.Resize(
                std::max(1, levelWidth / tileSize),
                std::max(1, levelHeight / tileSize),
                tileSize);
        }

        for (const JsonValue& layer : *layers)
        {
            const std::string type = StringMember(layer, "__type");
            if (type == "Tiles")
            {
                // 시각 타일은 현재 디버그/미래 렌더링용으로 id만 보관합니다.
                // 실제 충돌은 IntGrid_Collision 레이어가 기준입니다.
                ReadVisualTiles(layer, outWorld.tileMap);
            }

            if (type != "Entities")
            {
                continue;
            }

            const std::vector<JsonValue>* entities = ArrayMember(layer, "entityInstances");
            if (!entities)
            {
                continue;
            }

            for (const JsonValue& entity : *entities)
            {
                ReadEntity(entity, static_cast<float>(levelHeight), outWorld);
            }
        }

        if (outWorld.areas.empty())
        {
            // AreaZone을 하나도 찍지 않은 테스트 맵도 월드로 사용할 수 있게,
            // 레벨 전체를 T0 지역 하나로 등록합니다.
            const std::string levelId = StringMember(level, "identifier", "ldtk_level");
            outWorld.areas.push_back(rjm::Area{
                levelId,
                levelId,
                outWorld.worldBounds,
                outWorld.worldBounds,
                rjm::TraversalTier::T0
            });
        }

        return true;
    }
}

namespace rjm
{
    // Load:
    // - .ldtk 프로젝트 파일 또는 .ldtkl 외부 레벨 파일을 읽어 LdtkLoadedWorld를 채웁니다.
    // - 로딩 실패는 예외를 던지지 않고 false/outError로 돌려줘 WorldMap이 디버그 월드로 fallback할 수 있게 합니다.
    bool LdtkWorldLoader::Load(
        const std::string& path,
        LdtkLoadedWorld& outWorld,
        std::string* outError,
        const LdtkLoadOptions& options) const
    {
        std::string text;
        if (!ReadTextFile(path, text))
        {
            if (outError)
            {
                *outError = "Could not open LDtk file: " + path;
            }
            return false;
        }

        std::string error;
        JsonValue root;
        JsonParser parser(text);
        if (!parser.Parse(root, error))
        {
            if (outError)
            {
                *outError = error;
            }
            return false;
        }

        const JsonValue* level = SelectLevel(root, options);
        if (!level)
        {
            if (outError)
            {
                *outError = "Could not find requested LDtk level.";
            }
            return false;
        }

        JsonValue externalLevelRoot;
        if (!ArrayMember(*level, "layerInstances"))
        {
            // LDtk에서 external levels 옵션을 켜면 프로젝트 파일에는 레벨 본문이 없고
            // externalRelPath가 가리키는 별도 .ldtkl 파일에 layerInstances가 들어 있습니다.
            const std::string externalRelPath = StringMember(*level, "externalRelPath");
            if (!externalRelPath.empty())
            {
                const std::filesystem::path basePath(path);
                const std::filesystem::path externalPath = basePath.parent_path() / externalRelPath;

                std::string externalText;
                if (!ReadTextFile(externalPath.string(), externalText))
                {
                    if (outError)
                    {
                        *outError = "Could not open LDtk external level file: " + externalPath.string();
                    }
                    return false;
                }

                JsonParser externalParser(externalText);
                if (!externalParser.Parse(externalLevelRoot, error))
                {
                    if (outError)
                    {
                        *outError = error;
                    }
                    return false;
                }

                level = &externalLevelRoot;
            }
        }

        if (!BuildWorldFromLevel(*level, outWorld, error, options))
        {
            if (outError)
            {
                *outError = error;
            }
            return false;
        }

        return true;
    }
}
