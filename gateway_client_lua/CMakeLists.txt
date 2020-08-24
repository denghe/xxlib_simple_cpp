cmake_minimum_required(VERSION 3.10)
project(gateway_client_lua)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../xxlib
        ../lua-5.4.0/src
        #/usr/include/lua5.3/
        #/usr/include/luajit-2.1/
)

# lua 源代码文件列表
set(LUA_SRC
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lapi.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lcode.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lctype.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/ldebug.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/ldo.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/ldump.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lfunc.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lgc.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/llex.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lmem.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lobject.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lopcodes.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lparser.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lstate.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lstring.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/ltable.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/ltm.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lundump.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lvm.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lzio.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lauxlib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lbaselib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lcorolib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/ldblib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/liolib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lmathlib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/loadlib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/loslib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lstrlib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/ltablib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/lutf8lib.c
        ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/linit.c
        )
# 强制 lua *.c 以 c++ 方式编译以支持 c++ 异常 和析构
set_source_files_properties(${LUA_SRC} PROPERTIES LANGUAGE CXX)
# LUA_COMPILE_AS_CPP 告知 xx_lua.h 用的是 C++ 编译版 lua, 以去掉 extern C
# -flto 似乎能大幅度提升性能( 1/3 左右 )
add_definitions("-DLUA_COMPILE_AS_CPP -flto")

add_executable(gateway_client_lua
        main.cpp
        ${LUA_SRC}
        )

target_link_libraries(gateway_client_lua
        mimalloc
        pthread
        readline
        #lua5.3
        #luajit-5.1
        )
