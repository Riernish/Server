cmake_minimum_required(VERSION 3.10)
project(server)

set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} server.c my_server.h packet/packet.h packet/packet.c ID/ID.c ID/ID.h Error.c Error.h log.h)

add_executable(server_slave server_slave.c my_server.h packet/packet.h packet/packet.c Error.c Error.h log.h)

##############


project(client)

set(CMAKE_C_STANDARD 99)

add_executable(${PROJECT_NAME} client.c packet/packet.c ID/ID.c Error.c Error.h log.h)