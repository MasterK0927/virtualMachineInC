cmake_minimum_required(VERSION 3.15)
project(SimpleVM CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Options
option(SIMPLEVM_BUILD_TESTS "Build tests" OFF)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Sources
file(GLOB SIMPLEVM_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

add_executable(simplevm ${SIMPLEVM_SOURCES})

# Warnings
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(simplevm PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(simplevm PRIVATE /W4)
endif()
