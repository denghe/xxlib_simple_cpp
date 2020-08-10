cmake_minimum_required(VERSION 3.10)
project(simple_http_server)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../xxlib
)

add_executable(simple_http_server
        main.cpp
        server.cpp
        hlistener.cpp
        hpeer.cpp
        ../xxlib/http_parser.c
        )

target_link_libraries(simple_http_server
        pthread
        readline
        )
