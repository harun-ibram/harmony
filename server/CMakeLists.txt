add_executable(harmony_server
    src/main.cpp
)

target_link_libraries(harmony_server PRIVATE harmony_protocol)

if (WIN32)
  target_link_libraries(harmony_server PRIVATE ws2_32)
endif()
