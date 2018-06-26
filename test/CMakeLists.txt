cmake_minimum_required(VERSION 3.9)
project(morton-nd)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fconstexpr-steps=2147483647")


add_executable(morton-nd
        main.cpp
        ../morton-nd/include/mortonND_encoder.h)
