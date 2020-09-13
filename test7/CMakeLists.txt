cmake_minimum_required(VERSION 3.10)
project(test7)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ./
        ../xxlib
        ../lua-5.4.0/src
        #/usr/include/luajit-2.1/
)

set(LUA_SRC ${PROJECT_SOURCE_DIR}/../lua-5.4.0/src/onelua.c)
set_source_files_properties(${LUA_SRC} PROPERTIES LANGUAGE CXX)
add_definitions("-DMAKE_LIB -DLUA_USE_LINUX -DSOL_USING_CXX_LUA -flto")

add_executable(test7
        main.cpp
        ${LUA_SRC}
        )

target_link_libraries(test7
        dl
        pthread
        #luajit-5.1
        )
