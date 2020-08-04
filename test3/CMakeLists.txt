cmake_minimum_required(VERSION 3.16)
project(test3)

set(CMAKE_CXX_STANDARD 17)

#add_definitions("-O0 -g -Wall -Wno-implicit-fallthrough")

include_directories(
        ../xxlib
        ../
)

add_executable(test3
        main.cpp
        ../xxlib/ikcp.c
        )

target_link_libraries(test3
        pthread
        readline
        )
