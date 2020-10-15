cmake_minimum_required(VERSION 3.16)
project(simple_coroutine_test)

set(CMAKE_CXX_STANDARD 20)

#gcc10
#set(CMAKE_C_COMPILER gcc-10)
#set(CMAKE_CXX_COMPILER g++-10)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines")
#set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto")

#clang10 如果编译通过 但是编辑器飘红, 参看 readme.md
#release 编译模式得 -O2 否则 clang10 会崩
set(CMAKE_C_COMPILER clang-10)
set(CMAKE_CXX_COMPILER clang++-10)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fcoroutines-ts -stdlib=libc++ -v -O2")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -flto -fuse-ld=gold")

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
