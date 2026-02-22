cmake_minimum_required(VERSION 3.20)
project(Harmony VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# ── Dependencies via vcpkg ──────────────────────────────────────────
find_package(Boost REQUIRED COMPONENTS system)
find_package(nlohmann_json CONFIG REQUIRED)
# TODO 1a: Add find_package(unofficial-sqlite3 CONFIG REQUIRED) here
#          so the SQLite imported target is available to server/CMakeLists.txt

# ── Sub-projects ────────────────────────────────────────────────────
add_subdirectory(common)
add_subdirectory(server)
add_subdirectory(client)
add_subdirectory(tests)
