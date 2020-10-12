cmake_minimum_required(VERSION 3.10)
project(msvc_test2)

set(CMAKE_CXX_STANDARD 17)

include_directories(
	../xxlib
	../asio/asio/include
)

add_executable(msvc_test2
		../xxlib/ikcp.c
        main.cpp
        )

target_link_libraries(msvc_test2 pthread readline)
