cmake_minimum_required(VERSION 3.20)
project(basiclang)

set(CMAKE_CXX_STANDARD 20)

add_executable(bl src/main.cpp
        src/tokenization.h
        src/parser.h
        src/generation.h)
