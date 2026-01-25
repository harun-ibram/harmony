add_library(harmony_protocol
    src/protocol.cpp
)

target_include_directories(harmony_protocol
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_compile_features(harmony_protocol
    PUBLIC
        cxx_std_20
)
