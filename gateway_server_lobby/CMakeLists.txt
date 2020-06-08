cmake_minimum_required(VERSION 3.16)
project(gateway_server_lobby)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(gateway_server_lobby
        main.cpp
        server.cpp
        gpeer.cpp
        speer.cpp
        glistener.cpp
        slistener.cpp
        gpeer_aphandler.cpp
        gpeer_gphandler.cpp
        speer_aphandler.cpp
        speer_sphandler.cpp
        )

target_link_libraries(gateway_server_lobby pthread readline)
