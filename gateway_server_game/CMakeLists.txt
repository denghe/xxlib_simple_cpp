cmake_minimum_required(VERSION 3.16)
project(gateway_server_game)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(gateway_server_game
        main.cpp
        server.cpp
        peer.cpp
        lpeer.cpp
        dialer.cpp
        listener.cpp
        phandler.cpp
        aphandler.cpp
        gphandler.cpp
        )

target_link_libraries(gateway_server_game pthread readline)
