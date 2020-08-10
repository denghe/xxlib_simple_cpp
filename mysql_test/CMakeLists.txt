cmake_minimum_required(VERSION 3.10)
project(mysql_test)

set(CMAKE_CXX_STANDARD 17)
add_definitions("-Wall -Wno-implicit-fallthrough")
include_directories(
        ../xxlib
        ../
)

add_executable(mysql_test
        main.cpp)

target_link_libraries(mysql_test
        pthread
        readline
        mariadb
        )
