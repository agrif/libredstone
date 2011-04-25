cmake_minimum_required(VERSION 2.6)
project(redstone C)

option(BUILD_SHARED_LIBS "build shared libraries" ON)

add_subdirectory(src)
add_subdirectory(tools)
