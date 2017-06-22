cmake_minimum_required(VERSION 3.7)
project(abs_sock_srv)

set(CMAKE_C_STANDARD 99)

set(SOURCE_FILES main.c)
add_executable(abs_sock_srv ${SOURCE_FILES})