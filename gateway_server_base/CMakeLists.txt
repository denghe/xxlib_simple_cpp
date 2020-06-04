cmake_minimum_required(VERSION 3.16)
project(gateway_server_base)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(gateway_server_base main.cpp)

target_link_libraries(gateway_server_base pthread readline)
