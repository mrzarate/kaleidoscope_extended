cmake_minimum_required(VERSION 3.20)
project(kaleidoscope_extended)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(kaleidoscope_extended
    src/Lexer.cpp
    src/Parser.cpp
    src/Driver.cpp
    src/Main.cpp
)