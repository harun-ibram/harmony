#pragma once

#include <string>
#include <string_view>
#include <optional>

namespace harmony::proto {

/*
============================================================
Protocol Envelope (THE RULE)
============================================================

Every message you send over the network is a JSON object with
the SAME OUTER SHAPE (the "envelope"):

{
  "type": "...",           // required: what kind of message is this?
  "from": "...",           // optional: who sent it? (required for some types)
  "room": "...",           // optional: which room? (required for room messages)
  "to": "...",             // optional: for DMs later
  "body": { ... }          // optional: message-specific payload
}

Why do this?
- Server/client can ALWAYS read "type" first and know how to handle the message.
- "from/room/to" are routing fields, so they live at the top-level.
- "body" holds whatever data that specific message needs.

BEGINNER CHOICE (what we're doing now):
- One function per message type that returns the COMPLETE JSON string.
- No fancy abstractions yet.

Later upgrade:
- Replace manual string-building with a JSON library (same function signatures).
*/


// ============================================================
// 1) Message Types (strings you will literally send)
// ============================================================

/*
TODO:
- Decide and freeze your type strings for MVP.
- Keep them lowercase and simple.
- MVP set:
  - "login"
  - "join"
  - "chat"
  - "event"
  - "error"
*/

enum class Type {
    Login,
    Join,
    Chat,
    Event,
    Error,
    Unknown
};

/*
TODO:
- Map enum <-> string.
- These are used in parsing and building messages.
*/
Type type_from_string(std::string_view s);
std::string_view type_to_string(Type t);

struct Message {
    Type type = Type::Unknown;

    std::string from;
    std::string room;
    std::string to;

    std::string text;
};


// ============================================================
// 3) JSON escaping (important but can be postponed)
// ============================================================

/*
TODO (MVP):
- You can temporarily avoid escaping by only testing with simple text:
  no quotes (") and no newlines.

TODO (next):
- Implement json_escape(s) so text fields are safe:
  - replace \ with \\
  - replace " with \"
  - replace \n with \\n  (etc.)
*/
std::string json_escape(std::string_view s);


// ============================================================
// 4) "One function per message type" builders (COMPLETE JSON)
// ============================================================

/*
IMPORTANT:
- Each function returns the FULL envelope JSON string.
- Do not return just "body" fragments in this beginner version.

TODO for all builders:
- Build JSON using the envelope shape.
- Keep field order consistent for sanity when debugging.
- Decide which fields are required for each type (see notes below).
*/


// -------------------- LOGIN --------------------
/*
Envelope goal:
{
  "type":"login",
  "body": { "username":"..." }
}

TODO:
- Decide if username lives in:
  A) body.username  (recommended)
  B) envelope.from  (also possible)
Pick ONE and stick to it.

Note:
- For login, "from" may be empty until login is processed.
*/
std::string make_login(std::string_view username);


// -------------------- JOIN --------------------
/*
Envelope goal:
{
  "type":"join",
  "from":"<username>",     // recommended once logged in
  "room":"general"
}

TODO:
- Decide if join needs "from".
  - If server already knows the session username, you can omit "from".
  - If you want messages to be self-contained, include it.
Pick one approach and be consistent.
*/
std::string make_join(std::string_view from, std::string_view room);


// -------------------- CHAT --------------------
/*
Envelope goal:
{
  "type":"chat",
  "from":"nephis",
  "room":"general",
  "body": { "text":"hello" }
}

TODO:
- Put chat text inside body.text.
- Use json_escape(text) once implemented.
*/
std::string make_chat(std::string_view from,
                      std::string_view room,
                      std::string_view text);


// -------------------- EVENT --------------------
/*
Server -> client informational message.

Envelope goal:
{
  "type":"event",
  "body": { "text":"user joined" }
}

TODO:
- Keep it simple: body.text.
*/
std::string make_event(std::string_view text);


// -------------------- ERROR --------------------
/*
Server -> client error message.

Envelope goal:
{
  "type":"error",
  "body": { "text":"username already taken" }
}

TODO:
- Keep it simple: body.text.
- Later you can add body.code, etc.
*/
std::string make_error(std::string_view text);


// ============================================================
// 5) Parsing (turn received JSON string into Message)
// ============================================================

/*
TODO (MVP parsing):
- Implement a BASIC parser using string searching (find).
- Extract at least:
  - "type"
  - "from" (if present)
  - "room" (if present)
  - body.text (if present)

Rules:
- If "type" missing -> return nullopt
- If type unrecognized -> type = Unknown (or nullopt, your choice)
- If body.text missing for chat/error/event -> treat as invalid (your choice)

Later upgrade:
- Swap implementation to a JSON library.
- Keep the same signature so server/client code doesn't change.
*/
std::optional<Message> parse(std::string_view json);


// ============================================================
// 6) Validation helpers (optional, but helps server code)
// ============================================================

/*
TODO:
- Add simple "is this message complete enough?" checks.
- These make server/router code cleaner.
*/
bool is_valid_login(const Message& m);
bool is_valid_join(const Message& m);
bool is_valid_chat(const Message& m);


// ============================================================
// 7) Notes for later (do NOT implement now)
// ============================================================

/*
TODO (later):
- Add "req_id" so client can match responses to requests.
- Add "ts" timestamps (server-side is simplest).
- Add DMs:
  { "type":"dm", "from":"...", "to":"...", "body":{"text":"..."} }
- Add protocol versioning:
  { "v":1, ... }
*/

} // namespace harmony::proto
