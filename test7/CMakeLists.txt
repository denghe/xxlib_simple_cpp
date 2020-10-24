cmake_minimum_required(VERSION 3.10)
project(test7)

set(CMAKE_CXX_STANDARD 20)

#gcc10+
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")

#clang9+ 如果编译通过 但是编辑器飘红, 需要自己复制 /usr/lib/llvm 里面的 include 等内容到编辑器 .h cache 目录。具体参看 readme
set(CMAKE_C_COMPILER clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fcoroutines-ts -stdlib=libc++ -v")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -fuse-ld=gold")

include_directories(
        ./
        ../xxlib
        ../lua-5.4.x/src
        #/usr/include/luajit-2.1/
)

set(LUA_SRC ${PROJECT_SOURCE_DIR}/../lua-5.4.x/src/onelua.c)
set_source_files_properties(${LUA_SRC} PROPERTIES LANGUAGE CXX)
add_definitions("-DMAKE_LIB -DLUA_USE_LINUX -DSOL_USING_CXX_LUA")

add_executable(test7
        main.cpp
        ${LUA_SRC}
        )

target_link_libraries(test7
        dl
        pthread
        #luajit-5.1
        )
