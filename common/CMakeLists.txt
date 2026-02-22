# common – header-only library shared by server & client
add_library(harmony_common INTERFACE)

target_include_directories(harmony_common
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(harmony_common
    INTERFACE
        nlohmann_json::nlohmann_json
)
