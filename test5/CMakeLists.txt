cmake_minimum_required(VERSION 3.10)
project(test5)

set(CMAKE_CXX_STANDARD 17)

#add_definitions("-O0 -g -Wall -Wno-implicit-fallthrough")

include_directories(
        ../xxlib
        ../
)

add_executable(test5
        main.cpp
        ../xxlib/ikcp.c
        )

target_link_libraries(test5
        pthread
        readline
        )
