cmake_minimum_required(VERSION 3.10)
project(test2)

set(CMAKE_CXX_STANDARD 17)

#add_definitions("-O0 -g -Wall -Wno-implicit-fallthrough")

include_directories(
        ../xxlib
        ../
)

add_executable(test2
        main.cpp
        PKG_class_lite.cpp
        nanolog.h slog.h mylog.h)

target_link_libraries(test2
        pthread
        )
