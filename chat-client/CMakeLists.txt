cmake_minimum_required(VERSION 3.30)
project(chat_client C)

set(CMAKE_C_STANDARD 23)

add_executable(chat_client src/chat-client.c)
target_link_libraries(chat_client ncurses)
