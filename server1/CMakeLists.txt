cmake_minimum_required(VERSION 3.16)
project(server1)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(server1 main.cpp)

target_link_libraries(server1 pthread readline)