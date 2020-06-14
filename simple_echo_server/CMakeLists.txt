cmake_minimum_required(VERSION 3.16)
project(simple_echo_server)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(simple_echo_server
        main.cpp
        server.cpp
        listener.cpp
        peer.cpp
        )

target_link_libraries(simple_echo_server pthread readline)
