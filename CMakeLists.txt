cmake_minimum_required(VERSION 3.10)
project(all)

add_subdirectory(simple_echo_client)
add_subdirectory(simple_echo_server)
add_subdirectory(simple_echo_server_with_sqlite)

add_subdirectory(gateway_server_game)
add_subdirectory(gateway_server_gateway)
add_subdirectory(gateway_server_gateway_kcp)
add_subdirectory(gateway_server_lobby)

add_subdirectory(gateway_client_cpp)
add_subdirectory(gateway_client_lua)
add_subdirectory(gateway_tools_ping)
add_subdirectory(gateway_tools_dialtime)

add_subdirectory(simple_http_server)

add_subdirectory(logger_test)
add_subdirectory(mysql_test)

add_subdirectory(test1)
add_subdirectory(test2)
add_subdirectory(test3)
add_subdirectory(test4)
add_subdirectory(test5)
add_subdirectory(test6)
add_subdirectory(test7)

add_subdirectory(msvc_test1)
add_subdirectory(msvc_test2)