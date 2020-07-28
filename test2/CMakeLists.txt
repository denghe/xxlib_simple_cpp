cmake_minimum_required(VERSION 3.16)
project(test2)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../xxlib
        ../
)

add_executable(test2
        main.cpp
        PKG_class_lite.cpp
        nanolog.h)
