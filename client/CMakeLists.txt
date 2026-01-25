add_executable(harmony_client
    src/main.cpp
)

target_link_libraries(harmony_client PRIVATE harmony_protocol)

if (WIN32)
  target_link_libraries(harmony_client PRIVATE ws2_32)
endif()
