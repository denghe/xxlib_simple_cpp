cmake_minimum_required(VERSION 3.16)
project(test3)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(test3
        main.cpp
        )
