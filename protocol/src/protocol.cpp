#include <harmony/protocol.hpp>

#include <string>
#include <string_view>
#include <optional>

namespace harmony::proto {

Type type_from_string(std::string_view s)
{
    if (!s.compare("login")) return Type::Login;
    if (!s.compare("join")) return Type::Join;
    if (!s.compare("chat")) return Type::Chat;
    if (!s.compare("event")) return Type::Event;
    if (!s.compare("error")) return Type::Error;
    return Type::Unknown;
}

std::string_view type_to_string(Type t)
{
    switch (t)
    {
    case Type::Login:
        return "login";
    case Type::Join:
        return "join";
    case Type::Chat:
        return "chat";
    case Type::Event:
        return "event";
    case Type::Error:
        return "error";
    default:
        return "unkonwn";
    }
}


// ============================================================
// 2) JSON escaping (optional at first)
// ============================================================

std::string json_escape(std::string_view s)
{
    /*
    TODO (MVP):
    - For now, you can simply return std::string{s}
    - Later, implement proper escaping:
      - \" for quotes
      - \\ for backslashes
      - \\n for newlines

    IMPORTANT:
    - Keep the function even if it does nothing yet.
    */
    return std::string{s};
}


// ============================================================
// 3) Message builders (FULL envelope JSON)
// ============================================================

std::string make_login(std::string_view username)
{
    std::string msg;

    msg += "{";
    msg += "\"type\":\"login\",";
    msg += "\"body\":{";
    msg += "\"username\":\"";
    msg += username;
    msg += "\"}";
    msg += "}";

    return msg;
}


std::string make_join(std::string_view from, std::string_view room)
{
    std::string msg;

    msg += "{";
    msg += "\"type\":\"join\",";
    msg += "\"from\":\"";
    msg += from;
    msg += "\",";
    msg += "\"room\":\"";
    msg += room;
    msg += "\"";
    msg += "}";

    return msg;
}

std::string make_chat(std::string_view from,
                      std::string_view room,
                      std::string_view text)
{
    std::string msg;

    msg += "{";
    msg += "\"type\":\"chat\",";
    msg += "\"from\":\"";
    msg += from;
    msg += "\",";
    msg += "\"room\":\"";
    msg += room;
    msg += "\",";
    msg += "\"body\":{";
    msg += "\"text\":\"";
    msg += text;
    msg += "\"}";
    msg += "}";

    return msg;
}

std::string make_event(std::string_view text)
{
    std::string msg;

    msg += "{";
    msg += "\"type\":\"event\",";
    msg += "\"body\":{";
    msg += "\"text\":\"";
    msg += text;
    msg += "\"}";
    msg += "}";

    return msg;
}

std::string make_error(std::string_view text)
{
    std::string msg;

    msg += "{";
    msg += "\"type\":\"error\",";
    msg += "\"body\":{";
    msg += "\"text\":\"";
    msg += text;
    msg += "\"}";
    msg += "}";

    return msg;
}


// ============================================================
// 4) Parsing (JSON string -> Message struct)
// ============================================================

std::optional<Message> parse(std::string_view json)
{
    /*
    TODO (VERY IMPORTANT – DO THIS IN STAGES):

    Stage 1 (minimum viable parsing):
    - Extract "type"
    - Convert it using type_from_string
    - Fill Message.type
    - Ignore everything else

    Stage 2:
    - Extract "from" if present
    - Extract "room" if present

    Stage 3:
    - Extract body.text if present

    Rules:
    - If "type" is missing, return std::nullopt
    - If type is unknown, you may still return Message{Type::Unknown}

    Beginner tip:
    - Use string::find
    - Do NOT try to write a real JSON parser
    - This is about understanding flow, not JSON correctness
    */

    Message msg;

    // Extract type
    const std::string_view key = "\"type\":\"";

    size_t type_pos = json.find(key);
    if (type_pos == std::string_view::npos) {
        return std::nullopt;
    }

    type_pos += key.size();

    size_t type_end = json.find('"', type_pos);
    if (type_end == std::string_view::npos)
    {
        return std::nullopt;
    }

    std::string_view type_str = json.substr(type_pos, type_end - type_pos);

    msg.type = type_from_string(type_str);

    std::string json_str(json);
    switch (msg.type)
    {
    case harmony::proto::Type::Login:
        parseLogin(json_str, msg);
        break;
    case harmony::proto::Type::Chat:
        parseChat(json_str, msg);
        break;
    case harmony::proto::Type::Event:
        // parseEvent(json_str, msg);
        break;
    case harmony::proto::Type::Error:
        // parseError(json_str, msg);
        break;
    case harmony::proto::Type::Join:
        // parseJoin(json_str, msg);
        break;
    case harmony::proto::Type::Unknown:
        break;
    default:
        break;
    }
    return msg;
}


// ============================================================
// 5) Validation helpers (optional, can be added later)
// ============================================================

bool is_valid_login(const Message& m)
{
    /*
    TODO:
    - Check m.type == Type::Login
    - Check required fields are non-empty
    */
    return false;
}

bool is_valid_join(const Message& m)
{
    /*
    TODO:
    - Check m.type == Type::Join
    - Check room is non-empty
    */
    return false;
}

bool is_valid_chat(const Message& m)
{
    /*
    TODO:
    - Check m.type == Type::Chat
    - Check from, room, and text are non-empty
    */
    return false;
}

} // namespace harmony::proto
