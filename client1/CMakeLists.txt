cmake_minimum_required(VERSION 3.16)
project(client1)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(client1 main.cpp)

target_link_libraries(client1 pthread readline)