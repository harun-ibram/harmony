# Harmony

Harmony is a C++17 real-time chat project built with Boost.Asio, CMake, and vcpkg.

Current implementation includes:
- Length-prefixed TCP framing (`4-byte big-endian length + body`)
- JSON protocol v2 envelope helpers
- Async server/client core libraries
- Console client commands for login, DM, and group chat
- Integration test executable covering ping/login/DM/group flows

## Project Layout

- `common/` shared protocol + wire helpers (`harmony_common`)
- `server/` async TCP server (`harmony_server_core`, `harmony_server`)
- `client/` async TCP client (`harmony_client_core`, `harmony_client`)
- `tests/` integration tests (`ping_test`)

## Prerequisites

- CMake 3.20+
- A C++17 compiler
- vcpkg with `VCPKG_ROOT` environment variable set

This repository uses these vcpkg dependencies:
- `boost-asio`
- `boost-system`
- `boost-beast`
- `nlohmann-json`
- `sqlite3` (declared for upcoming persistence work)

## Configure & Build

From the repository root:

```bash
cmake --preset default
cmake --build --preset default
```

Release build:

```bash
cmake --preset release
cmake --build --preset release
```

You can also build specific targets:

```bash
cmake --build --preset default --target harmony_server
cmake --build --preset default --target harmony_client
cmake --build --preset default --target ping_test
```

## Running

### 1) Start the server

Default port is `9876`.

```bash
# from a typical single-config build tree
./build/server/harmony_server

# Windows example
.\build\server\harmony_server.exe
```

Optional port argument:

```bash
./build/server/harmony_server 9000
```

### 2) Start one or more clients

```bash
# host defaults to 127.0.0.1, port defaults to 9876
./build/client/harmony_client

# explicit host/port
./build/client/harmony_client 127.0.0.1 9876
```

Windows example:

```powershell
.\build\client\harmony_client.exe
```

If your generator is multi-config (for example Visual Studio), executables may be under `Debug/` or `Release/` subfolders.

## Console Client Commands

Inside `harmony_client`:

- `/login <username>`
- `/dm <user_id> <message>`
- `/join <group_id>`
- `/leave <group_id>`
- `/group <group_id> <message>`
- `/ping`
- `/quit`

## Protocol Notes

Harmony uses framed TCP messages. The body is currently UTF-8 JSON for protocol v2.

Envelope fields used by helpers include:
- `v` protocol version
- `t` message type
- `id` message id
- `reply_to` correlation id for responses
- `from`, `to`, `body`, `ok`, `error`, `ts`

Implemented message types include:
- `login.req`, `login.res`
- `ping`, `pong`
- `dm.send`, `dm.msg`
- `group.join`, `group.leave`, `group.send`, `group.msg`
- `error`

## Tests

Build and run integration tests:

```bash
cmake --build --preset default --target ping_test
./build/tests/ping_test
```

Windows example:

```powershell
.\build\tests\ping_test.exe
```

The test executable currently runs:
- JSON ping -> pong
- Login flow
- DM routing (Alice -> Bob)
- Group messaging

## Current Status / In-Progress Work

There are TODO markers in the code for upcoming persistence features:
- SQLite package wiring in CMake
- Stable user identities backed by DB
- Offline DM delivery
- Group membership persistence
- History request/response message types and handlers

So this README reflects the **current working behavior**, not the planned final feature set.
