cmake_minimum_required(VERSION 3.10)
project(gateway_client_lua)
set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../xxlib
        /usr/include/luajit-2.1
)
add_definitions("-DUSING_LUAJIT")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -fuse-ld=gold")
add_executable(gateway_client_lua
        main.cpp
        FileExts_class_lite.cpp
        Objs_class_lite.cpp
        )
target_link_libraries(gateway_client_lua
        dl
        pthread
        readline
        luajit-5.1
        )

## lua 5.4
#include_directories(
#        ../xxlib
#        ../lua-5.4.x/src
#)
#set(LUA_SRC ${PROJECT_SOURCE_DIR}/../lua-5.4.x/src/onelua.c)
#set_source_files_properties(${LUA_SRC} PROPERTIES LANGUAGE CXX)
#add_definitions("-DMAKE_LIB -DLUA_USE_LINUX")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -fuse-ld=gold")
#add_executable(gateway_client_lua
#        main.cpp
#        FileExts_class_lite.cpp
#        Objs_class_lite.cpp
#        ${LUA_SRC}
#        )
#target_link_libraries(gateway_client_lua
#        dl
#        pthread
#        readline
#        )