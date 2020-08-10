cmake_minimum_required(VERSION 3.10)
project(simple_echo_client)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(simple_echo_client
        main.cpp
        client.cpp
        dialer.cpp
        peer.cpp
        )

target_link_libraries(simple_echo_client pthread readline)
