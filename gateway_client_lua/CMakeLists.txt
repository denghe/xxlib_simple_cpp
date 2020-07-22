cmake_minimum_required(VERSION 3.16)
project(gateway_client_lua)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(gateway_client_lua main.cpp xx_luafunc.h)

target_link_libraries(gateway_client_lua pthread readline
        lua5.3
         #luajit-5.1
        )
