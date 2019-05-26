cmake_minimum_required(VERSION 3.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
project(Sorter)
add_executable(${PROJECT_NAME} "main.cpp")
