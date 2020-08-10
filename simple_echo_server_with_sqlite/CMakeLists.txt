cmake_minimum_required(VERSION 3.10)
project(simple_echo_server_with_sqlite)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(simple_echo_server_with_sqlite
        main.cpp
        server.cpp
        listener.cpp
        peer.cpp
        )

target_link_libraries(simple_echo_server_with_sqlite
        pthread
        readline
        sqlite3
        mariadb
        )
