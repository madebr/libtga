cmake_minimum_required(VERSION 3.0)

project(libtga VERSION 1.0.1)

option(BUILD_SHARED "Build libtga as a shared library (default OFF)" OFF)
option(BUILD_EXAMPLES "Build examples (default OFF)" OFF)
option(TGA_DEBUG "Enable debug definitions in libtga" OFF)

set(CMAKE_C_FLAGS "-Wall -pedantic -Wextra -std=c99")

set(LIBTGA_PROJECT_PATH "${CMAKE_CURRENT_LIST_DIR}")

add_subdirectory(src)

if(BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
