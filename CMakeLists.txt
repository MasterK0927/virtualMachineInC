cmake_minimum_required(VERSION 3.15)
project(SimpleVM CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core library
set(VMCORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CPU.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Decoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Instance.cpp
)

add_library(vmcore ${VMCORE_SOURCES})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(vmcore PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(vmcore PRIVATE /W4)
endif()

# VM application
add_executable(vm_app ${CMAKE_CURRENT_SOURCE_DIR}/apps/vm/main.cpp)
target_link_libraries(vm_app PRIVATE vmcore)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(vm_app PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(vm_app PRIVATE /W4)
endif()

# Assembler application
add_executable(asm_app ${CMAKE_CURRENT_SOURCE_DIR}/apps/asm/main.cpp)
target_link_libraries(asm_app PRIVATE vmcore)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(asm_app PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(asm_app PRIVATE /W4)
endif()
