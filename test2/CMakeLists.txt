cmake_minimum_required(VERSION 3.16)
project(test2)

set(CMAKE_CXX_STANDARD 17)

include_directories(../xxlib)

add_executable(test2
        main.cpp
        server.cpp
        listener.cpp
        peer.cpp
        )

target_link_libraries(test2 pthread readline sqlite3 mariadb)
