cmake_minimum_required(VERSION 3.10)
project(gateway_tools_ping)

set(CMAKE_CXX_STANDARD 17)

#add_definitions("-O0 -g -Wall -Wno-implicit-fallthrough")

include_directories(
        ../xxlib
        ../
)

add_executable(gateway_tools_ping
        main.cpp
        ../xxlib/ikcp.c
        )

target_link_libraries(gateway_tools_ping
        pthread
        readline
        )
