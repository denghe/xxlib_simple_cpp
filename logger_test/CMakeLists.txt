cmake_minimum_required(VERSION 3.16)
project(logger_test)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ../xxlib
        ../
)

add_executable(logger_test
        main.cpp)

target_link_libraries(logger_test
        pthread
        )
