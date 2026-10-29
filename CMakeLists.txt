cmake_minimum_required(VERSION 3.20)
project(myDeque CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(GTest CONFIG REQUIRED)
include(CTest)

add_library(myDeque INTERFACE)

target_include_directories(myDeque INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

add_subdirectory(tests)
add_subdirectory(examples)
