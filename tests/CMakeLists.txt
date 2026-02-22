# Day 2 + Day 3 integration tests – uses real server/client core libraries
# TODO 13: After SqliteStore is linked via harmony_server_core, tests automatically
#          get access to it.  No extra link changes needed here.
add_executable(ping_test
    ping_test.cpp
)

target_link_libraries(ping_test
    PRIVATE
        harmony_server_core
        harmony_client_core
)
