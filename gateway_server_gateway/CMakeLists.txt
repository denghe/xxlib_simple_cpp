cmake_minimum_required(VERSION 3.16)
project(gateway_server_gateway)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(gateway_server_gateway
        main.cpp
        server.cpp
        peer.cpp
        listener.cpp
        dialer.cpp
        cpeer.cpp
        speer.cpp
        pingtimer.cpp
        )

target_link_libraries(gateway_server_gateway
        pthread
        readline
        )
