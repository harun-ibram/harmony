# server core library (reusable by tests)
add_library(harmony_server_core STATIC
    src/server.cpp
    src/session.cpp
    # TODO 3o: Add src/harmony_store.cpp here once the file is created
)

target_include_directories(harmony_server_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(harmony_server_core
    PUBLIC
        harmony_common
        Boost::system
        # TODO 1b: Add unofficial::sqlite3::sqlite3 here to link SQLite
)

if(WIN32)
    target_link_libraries(harmony_server_core PUBLIC ws2_32 mswsock)
endif()

# server executable
add_executable(harmony_server
    src/main.cpp
)

target_link_libraries(harmony_server
    PRIVATE
        harmony_server_core
)
