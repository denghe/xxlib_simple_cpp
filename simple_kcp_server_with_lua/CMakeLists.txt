cmake_minimum_required(VERSION 3.16)
project(simple_kcp_server_with_lua)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

#sudo apt-get install libc++-10-dev libc++abi-10-dev

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -fcoroutines-ts -v")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -fuse-ld=gold")

include_directories(
        ../xxlib
        ../lua-5.4.x/src
        )

set(LUA_SRC ${PROJECT_SOURCE_DIR}/../lua-5.4.x/src/onelua.c)
set_source_files_properties(${LUA_SRC} PROPERTIES LANGUAGE CXX)
add_definitions("-DMAKE_LIB -DLUA_USE_LINUX")

add_executable(simple_kcp_server_with_lua
        ../xxlib/ikcp.c
        ${LUA_SRC}
        main.cpp
        cpeer.cpp
        listener.cpp
        server.cpp
        )


#target_link_directories(simple_kcp_server_with_lua       )

target_link_libraries(simple_kcp_server_with_lua
        dl
        pthread
        readline
        #coroutines
        )
