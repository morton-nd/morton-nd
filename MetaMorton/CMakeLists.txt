cmake_minimum_required(VERSION 3.9)
project(MetaMorton)

set(CMAKE_CXX_STANDARD 14)

add_executable(MetaMorton
        array_builder.h
        main.cpp
        mortoncode.h)
