cmake_minimum_required(VERSION 3.12.0)
project(inras VERSION 0.0.1 LANGUAGES C CXX)

# Include directories for using inras.
set(INRAS_INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include")

# Implementation
add_subdirectory(lib)