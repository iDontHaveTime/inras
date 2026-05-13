# lib: Contains the implementation for inras
# This file is for defining the libraries that the project uses.

# C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Default to shared libraries.
option(BUILD_SHARED_LIBS "Should inras be built as shared libraries" ON)

# Can be made global as every library should include this.
include_directories(${INRAS_INCLUDE_DIRS})

add_compile_options(-Wall -Wextra)

# Instruction library.
file(GLOB AssemblerSources "Assembler/*.cpp")
add_library(inras ${AssemblerSources})

file(GLOB TestSources "Tests/*.cpp")
add_executable(TestAsm ${TestSources})
target_link_libraries(TestAsm PUBLIC inras)