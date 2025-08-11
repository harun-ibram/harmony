cmake_minimum_required(VERSION 3.16)
project(harmony)

set(CMAKE_CXX_STANDARD 17)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
find_package(Boost REQUIRED COMPONENTS system)

add_executable(harmony
    src/main.cpp
    src/ChatWindow.cpp
    src/Client.cpp
    src/Server.cpp
)

target_link_libraries(harmony
    Qt6::Widgets
    Boost::system
)
