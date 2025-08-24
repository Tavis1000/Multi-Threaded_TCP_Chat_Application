cmake_minimum_required(VERSION 3.10)
project(ChatApp)

set(CMAKE_CXX_STANDARD 11)

add_executable(server Server.cpp)
add_executable(client Client.cpp)
