cmake_minimum_required(VERSION 3.30)
project(chat_server C)

set(CMAKE_C_STANDARD 23)

add_executable(chat_server src/chat-server.c)
