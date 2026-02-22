# client core library (reusable by tests)
add_library(harmony_client_core STATIC
    src/client.cpp
)

target_include_directories(harmony_client_core
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(harmony_client_core
    PUBLIC
        harmony_common
        Boost::system
)

if(WIN32)
    target_link_libraries(harmony_client_core PUBLIC ws2_32 mswsock)
endif()

# client executable (console client for Day 1–2; Qt comes on Day 4)
add_executable(harmony_client
    src/main.cpp
)

target_link_libraries(harmony_client
    PRIVATE
        harmony_client_core
)
