cmake_minimum_required(VERSION 3.16)
project(simple_coroutine_test)

set(CMAKE_CXX_STANDARD 20)

#gcc10+
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")

#clang9+ 如果编译通过 但是编辑器飘红, 需要自己复制 /usr/lib/llvm 里面的 include 等内容到编辑器 .h cache 目录。具体参看 readme
#set(CMAKE_C_COMPILER clang)
#set(CMAKE_CXX_COMPILER clang++)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Xclang -fcoroutines-ts -stdlib=libc++ -v")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -fuse-ld=gold")

include_directories(
        ../xxlib
        )

add_executable(simple_coroutine_test
        main.cpp
        )

target_link_libraries(simple_coroutine_test
        dl
        pthread
        readline
        )
