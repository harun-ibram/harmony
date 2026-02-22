#pragma once
// ─────────────────────────────────────────────────────────────────────
// Harmony – common/include/harmony/protocol.hpp
//
// Day 2 JSON envelope definition, message-type constants, and
// parse / serialize helpers.  The wire framing (4-byte length prefix)
// is unchanged – only the *body bytes* are now UTF-8 JSON.
// ─────────────────────────────────────────────────────────────────────

#ifndef HARMONY_PROTOCOL_HPP
#define HARMONY_PROTOCOL_HPP

#include <nlohmann/json.hpp>
#include <string>
#include <cstdint>
#include <chrono>
#include <random>
#include <sstream>
#include <iomanip>

namespace harmony::proto {

using json = nlohmann::json;

// ── Protocol version ───────────────────────────────────────────────
constexpr int PROTOCOL_VERSION = 2;

// ── Message type constants ─────────────────────────────────────────

namespace type {
    // Connection / auth
    constexpr const char* LOGIN_REQ     = "login.req";
    constexpr const char* LOGIN_RES     = "login.res";

    // Legacy ping (kept for backward compat / health checks)
    constexpr const char* PING          = "ping";
    constexpr const char* PONG          = "pong";

    // Direct messages
    constexpr const char* DM_SEND       = "dm.send";
    constexpr const char* DM_MSG        = "dm.msg";

    // Group chat
    constexpr const char* GROUP_JOIN    = "group.join";
    constexpr const char* GROUP_LEAVE   = "group.leave";
    constexpr const char* GROUP_SEND    = "group.send";
    constexpr const char* GROUP_MSG     = "group.msg";

    // Generic error
    constexpr const char* ERROR_MSG     = "error";

    // TODO 10a: Add history message types:
    //   constexpr const char* DM_HISTORY_REQ    = "dm.history.req";
    //   constexpr const char* DM_HISTORY_RES    = "dm.history.res";
    //   constexpr const char* GROUP_HISTORY_REQ = "group.history.req";
    //   constexpr const char* GROUP_HISTORY_RES = "group.history.res";
}

// ── Unique ID generation ───────────────────────────────────────────

/// Generate a simple unique message id (hex timestamp + random suffix).
inline std::string make_id()
{
    static thread_local std::mt19937 rng{std::random_device{}()};
    auto now = std::chrono::system_clock::now().time_since_epoch();
    auto ms  = std::chrono::duration_cast<std::chrono::milliseconds>(now).count();

    std::uniform_int_distribution<int> dist(0, 0xFFFF);
    std::ostringstream oss;
    oss << std::hex << ms << "-" << std::setw(4) << std::setfill('0') << dist(rng);
    return oss.str();
}

// ── Timestamp helper ───────────────────────────────────────────────

/// Current time in milliseconds since epoch.
inline std::int64_t now_ms()
{
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()).count();
}

// ── Envelope helpers ───────────────────────────────────────────────
//
//    v        (int)              – protocol version
//    t        (string)           – message type
//    id       (string)           – sender-generated unique id
//    reply_to (string, optional) – id of the request this responds to
//    from     (object, optional) – { "user_id": "…", "username": "…" }
//    to       (object, optional) – { "user_id": "…" } or { "group_id": "…" }
//    body     (object, optional) – type-specific payload
//    ok       (bool,   optional) – present on responses
//    error    (object, optional) – { "code": "…", "message": "…" }
//    ts       (int64,  optional) – server timestamp (ms)
//
//  We don't use a C++ struct because the envelope is intentionally
//  flexible (different types carry different optional fields).
//  Helper functions build / inspect envelopes.
// ───────────────────────────────────────────────────────────────────

/// Create a minimal envelope with version, type, and id.
inline json make_envelope(const std::string& msg_type)
{
    return {
        {"v",  PROTOCOL_VERSION},
        {"t",  msg_type},
        {"id", make_id()}
    };
}

/// Create a response envelope that references a request id.
inline json make_response(const std::string& msg_type,
                          const std::string& request_id,
                          bool ok = true)
{
    json env = make_envelope(msg_type);
    env["reply_to"] = request_id;
    env["ok"]       = ok;
    return env;
}

/// Create an error envelope referencing a request id.
inline json make_error(const std::string& request_id,
                       const std::string& code,
                       const std::string& message)
{
    json env = make_envelope(type::ERROR_MSG);
    env["reply_to"] = request_id;
    env["ok"]       = false;
    env["error"]    = {{"code", code}, {"message", message}};
    return env;
}

/// Safe extraction helpers (return defaults on missing keys).
inline std::string get_type(const json& env)
{
    return env.value("t", "");
}

inline std::string get_id(const json& env)
{
    return env.value("id", "");
}

inline std::string get_reply_to(const json& env)
{
    return env.value("reply_to", "");
}

/// Parse a raw JSON string.  Returns a pair {json, error_string}.
/// On failure, json is null and error_string describes the problem.
inline std::pair<json, std::string> parse(const std::string& raw)
{
    try {
        json j = json::parse(raw);
        if (!j.is_object())
            return {json{}, "envelope must be a JSON object"};
        if (!j.contains("t"))
            return {json{}, "envelope missing 't' (type) field"};
        return {std::move(j), ""};
    } catch (const json::parse_error& e) {
        return {json{}, std::string("JSON parse error: ") + e.what()};
    }
}

/// Serialize an envelope to a string (compact, no pretty-print).
inline std::string serialize(const json& env)
{
    return env.dump(-1);
}

// ── Convenience builders for common message types ──────────────────

/// Build a login.req envelope.
inline json login_req(const std::string& username)
{
    json env = make_envelope(type::LOGIN_REQ);
    env["body"] = {{"username", username}};
    return env;
}

/// Build a dm.send envelope.
inline json dm_send(const std::string& to_user_id, const std::string& text)
{
    json env = make_envelope(type::DM_SEND);
    env["to"]   = {{"user_id", to_user_id}};
    env["body"] = {{"text", text}};
    return env;
}

/// Build a group.join envelope.
inline json group_join(const std::string& group_id)
{
    json env = make_envelope(type::GROUP_JOIN);
    env["to"] = {{"group_id", group_id}};
    return env;
}

/// Build a group.leave envelope.
inline json group_leave(const std::string& group_id)
{
    json env = make_envelope(type::GROUP_LEAVE);
    env["to"] = {{"group_id", group_id}};
    return env;
}

/// Build a group.send envelope.
inline json group_send(const std::string& group_id, const std::string& text)
{
    json env = make_envelope(type::GROUP_SEND);
    env["to"]   = {{"group_id", group_id}};
    env["body"] = {{"text", text}};
    return env;
}

// TODO 10b: Add convenience builders for history requests:
//   inline json dm_history_req(const std::string& peer_user_id, int limit) {
//       json env = make_envelope(type::DM_HISTORY_REQ);
//       env["body"] = {{"peer_user_id", peer_user_id}, {"limit", limit}};
//       return env;
//   }
//   inline json group_history_req(const std::string& group_id, int limit) {
//       json env = make_envelope(type::GROUP_HISTORY_REQ);
//       env["body"] = {{"group_id", group_id}, {"limit", limit}};
//       return env;
//   }

} // namespace harmony::proto

#endif // HARMONY_PROTOCOL_HPP
