cmake_minimum_required(VERSION 3.16)
project(gateway_client_cpp)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(gateway_client_cpp
        main.cpp
        client.cpp
        dialer.cpp
        peer.cpp
        )

target_link_libraries(gateway_client_cpp pthread readline)
