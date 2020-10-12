cmake_minimum_required(VERSION 3.10)
project(simple_kcp_server_with_lua)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../xxlib
        ../lua-5.4.x/src
        )

set(LUA_SRC ${PROJECT_SOURCE_DIR}/../lua-5.4.x/src/onelua.c)
set_source_files_properties(${LUA_SRC} PROPERTIES LANGUAGE CXX)
add_definitions("-DMAKE_LIB -DLUA_USE_LINUX -flto")

add_executable(simple_kcp_server_with_lua
        ../xxlib/ikcp.c
        ${LUA_SRC}
        main.cpp
        cpeer.cpp
        listener.cpp
        server.cpp
        )

target_link_libraries(simple_kcp_server_with_lua
        dl
        pthread
        readline
        )
