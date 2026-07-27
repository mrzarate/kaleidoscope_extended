cmake_minimum_required(VERSION 3.20)
project(kaleidoscope_extended)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-rtti -fno-exceptions")

set(LLVM_DIR /usr/lib/llvm-21/lib/cmake/llvm)
find_package(LLVM REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})
include_directories(include)

llvm_map_components_to_libnames(LLVM_LIBS core support irreader)

add_executable(kaleidoscope_extended
    src/frontend/Lexer.cpp
    src/frontend/Parser.cpp
    src/frontend/TypeChecker.cpp
    src/codegen/CodeGen.cpp
    src/Driver.cpp
    src/Main.cpp
)

target_link_libraries(kaleidoscope_extended ${LLVM_LIBS})