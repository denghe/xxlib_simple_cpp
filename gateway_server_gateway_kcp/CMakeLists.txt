cmake_minimum_required(VERSION 3.16)
project(gateway_server_gateway_kcp)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(gateway_server_gateway_kcp
        main.cpp
        server.cpp
        peer.cpp
        listener.cpp
        dialer.cpp
        cpeer.cpp
        speer.cpp
		pingtimer.cpp

		kpeer.cpp
        ../xxlib/ikcp.c
        )

target_link_libraries(gateway_server_gateway_kcp
        pthread
        readline
        )
