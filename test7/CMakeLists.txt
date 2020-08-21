cmake_minimum_required(VERSION 3.10)
project(test7)

set(CMAKE_CXX_STANDARD 17)

#add_definitions("-O0 -g -Wall -Wno-implicit-fallthrough")

include_directories(
        ./
        ../xxlib
        #/usr/include/lua5.3/
        /usr/include/luajit-2.1/
)

add_executable(test7
        main.cpp
        )

target_link_libraries(test7
        pthread
        #lua5.3
        luajit-5.1
        )
