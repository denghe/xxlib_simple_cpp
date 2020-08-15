cmake_minimum_required(VERSION 3.10)
project(test2)

set(CMAKE_CXX_STANDARD 17)

add_definitions("-mno-fma")

include_directories(
        ../xxlib
        ../
)

add_executable(test2
        main.cpp
        PKG_class_lite.cpp)

target_link_libraries(test2
        pthread
        #mimalloc
        )
