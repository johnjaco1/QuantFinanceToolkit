cmake_minimum_required(VERSION 3.12)
project(QuantFinanceToolkit LANGUAGES CXX)

# Require modern C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build examples.cpp into an executable
add_executable(examples
    examples.cpp
)

# Include headers
target_include_directories(examples PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)

# Compiler warnings (optional but helpful)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(examples PRIVATE -Wall -Wextra -pedantic)
elseif (CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(examples PRIVATE /W4)
endif()
