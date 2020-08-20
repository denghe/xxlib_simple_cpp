cmake_minimum_required(VERSION 3.10)
project(gateway_client_lua)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../xxlib
        #/usr/include/lua5.3/
        /usr/include/luajit-2.1/
)

add_executable(gateway_client_lua
        main.cpp
        )

target_link_libraries(gateway_client_lua
        pthread
        readline
        #mimalloc
        #lua5.3
        luajit-5.1
        )
