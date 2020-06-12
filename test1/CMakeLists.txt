cmake_minimum_required(VERSION 3.16)
project(test1)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(test1 main.cpp)

target_link_libraries(test1 pthread readline sqlite3 mariadb)
