cmake_minimum_required(VERSION 3.16)
project(test1)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(test1
        ../xxlib/ikcp.c
        main.cpp
        client.cpp
        coro.cpp
        coromain.cpp
        resolver.cpp
        listener.cpp
        peer.cpp
        )

target_link_libraries(test1
        pthread
        uv
        )
