cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} udp/server.c my_server.h packet/packet.h packet/packet.c ID/ID.c ID/ID.h Error.c Error.h log.h log.c)

add_executable(server_slave udp/server_slave.c my_server.h packet/packet.h packet/packet.c Error.c Error.h log.h log.c)

##############


project(client)

set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} udp/client.c packet/packet.c ID/ID.c Error.c Error.h log.h log.c)

##############


project(client_tcp)

set(CMAKE_C_STANDARD 99)

add_executable(client_tcp tcp/client_tcp.c packet/packet.c ID/ID.c Error.c Error.h log.h log.c ID/ID_TCP.c)

##############

project(server_tcp)

set(CMAKE_C_STANDARD 99)

add_executable(server_tcp tcp/server_tcp.c my_server.h packet/packet.h packet/packet.c ID/ID.c ID/ID.h Error.c Error.h log.h log.c ID/ID_TCP.c)

add_executable(server_slave_tcp tcp/server_slave_tcp.c my_server.h packet/packet.h packet/packet.c Error.c Error.h log.h log.c ID/ID_TCP.c)
