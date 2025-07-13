cmake_minimum_required(VERSION 3.15)
project(SimpleVM 
    VERSION 1.0.0
    DESCRIPTION "A complete virtual machine implementation with assembler and GUI debugger"
    LANGUAGES CXX
)

# C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type default
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Core library
set(VMCORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/CPU.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Decoder.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Instance.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Bus.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ConsoleDevice.cpp
)

add_library(vmcore ${VMCORE_SOURCES})

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(vmcore PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(vmcore PRIVATE /W4)
endif()

# Simple tests (no external framework)
add_executable(vm_tests ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_vm.cpp)
target_link_libraries(vm_tests PRIVATE vmcore)
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(vm_tests PRIVATE -Wall -Wextra -Wpedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(vm_tests PRIVATE /W4)
endif()

# Enable CTest and register tests
include(CTest)
if (BUILD_TESTING)
    add_test(NAME vm_tests COMMAND vm_tests)
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

# Install rules
include(GNUInstallDirs)
install(TARGETS vm_app asm_app
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(TARGETS vmcore
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include/vm/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vm
    FILES_MATCHING PATTERN "*.hpp"
)

# CPack packaging (TGZ)
set(CPACK_PACKAGE_NAME "simplevm")
set(CPACK_PACKAGE_VENDOR "SimpleVM Project")
set(CPACK_PACKAGE_VERSION "0.1.0")
set(CPACK_GENERATOR "TGZ")
include(CPack)

# Optional GUI (SDL2) - Disabled by default to avoid network dependencies
option(BUILD_GUI "Build SDL2 GUI frontend" OFF)
if (BUILD_GUI)
    message(STATUS "GUI build requested - checking for SDL2...")
    find_package(SDL2 QUIET)
    if (SDL2_FOUND)
        message(STATUS "SDL2 found - GUI will be built with FetchContent for ImGui")
        include(FetchContent)
        set(FETCHCONTENT_QUIET OFF)  # Show progress
        
        # Set timeout for fetch operations
        set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
        
        # Fetch Dear ImGui (header-only + backends built as part of target)
        FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG v1.90.8
            GIT_SHALLOW TRUE
            TIMEOUT 30
        )
        
        # Check if imgui is already available
        FetchContent_GetProperties(imgui)
        if(NOT imgui_POPULATED)
            message(STATUS "Downloading Dear ImGui...")
            FetchContent_Populate(imgui)
        endif()

        set(IMGUI_DIR ${imgui_SOURCE_DIR})
        set(IMGUI_BACKENDS
            ${IMGUI_DIR}/backends/imgui_impl_sdl2.cpp
            ${IMGUI_DIR}/backends/imgui_impl_sdlrenderer2.cpp
        )

        add_executable(vm_gui
            ${CMAKE_CURRENT_SOURCE_DIR}/apps/gui/main.cpp
            ${CMAKE_CURRENT_SOURCE_DIR}/apps/gui/GuiApp.cpp
            ${IMGUI_DIR}/imgui.cpp
            ${IMGUI_DIR}/imgui_draw.cpp
            ${IMGUI_DIR}/imgui_tables.cpp
            ${IMGUI_DIR}/imgui_widgets.cpp
            ${IMGUI_DIR}/misc/cpp/imgui_stdlib.cpp
            ${IMGUI_BACKENDS}
        )
        target_include_directories(vm_gui PRIVATE ${IMGUI_DIR} ${IMGUI_DIR}/backends)
        target_link_libraries(vm_gui PRIVATE vmcore SDL2::SDL2)
        if (CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
            target_compile_options(vm_gui PRIVATE -Wall -Wextra -Wpedantic)
        elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
            target_compile_options(vm_gui PRIVATE /W4)
        endif()
        message(STATUS "GUI target configured successfully")
    else()
        message(WARNING "SDL2 not found; vm_gui will not be built. Install libsdl2-dev or disable BUILD_GUI.")
    endif()
else()
    message(STATUS "GUI build disabled. Use -DBUILD_GUI=ON to enable.")
endif()
