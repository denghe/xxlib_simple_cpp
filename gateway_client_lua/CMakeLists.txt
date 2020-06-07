cmake_minimum_required(VERSION 3.16)
project(gateway_client_lua)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(gateway_client_lua main.cpp)

target_link_libraries(gateway_client_lua pthread readline luajit-5.1)
