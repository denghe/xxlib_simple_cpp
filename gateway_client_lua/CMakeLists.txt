cmake_minimum_required(VERSION 3.10)
project(gateway_client_lua)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../xxlib
        ../lua-5.4.0/src
        #/usr/include/luajit-2.1
)

# 强制以 c++ 方式编译 lua 以支持 c++ 异常 和析构
# 通过 MAKE_LIB 宏 告知 onelua.c 编译的仅仅是 LIB，xx_lua.h 也利用这个宏去掉 extern C
# -flto 似乎能大幅度提升性能( 1/3 左右 )
set(LUA_SRC ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/onelua.c)
set_source_files_properties(${LUA_SRC} PROPERTIES LANGUAGE CXX)
add_definitions("-DMAKE_LIB -DLUA_USE_LINUX -flto")

add_executable(gateway_client_lua
        main.cpp
        FileExts_class_lite.cpp
        ${LUA_SRC}
        )

target_link_libraries(gateway_client_lua
        dl
        pthread
        readline
        #luajit-5.1
        )
