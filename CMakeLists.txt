# Root CMake File

cmake_minimum_required(VERSION 3.18)

# vcpkg toolchain
include(cmake/vcpkg.cmake)

# msvc configuration cmake
include(cmake/msvc_settings.cmake)

# Project Name and Language
project(physics_eg
        LANGUAGES CXX)

# Project configuration interface
add_library(project_configuration INTERFACE)

# Compile using C++20 features
target_compile_features(project_configuration
    INTERFACE
        cxx_std_20)

# If we are in DEBUG mode
target_compile_definitions(project_configuration
    INTERFACE
        $<$<CONFIG:Debug>:DEBUG _DEBUG>)

# Load MSVC specific settings
set(MSVC_CXX_LATEST ON)
set(MSVC_WIN32 ON)
set_msvc_project_configuration(project_configuration)

# Place build output to build/bin directory
set(EXECUTABLE_OUTPUT_PATH "${CMAKE_BINARY_DIR}/bin/")

# Source Directory
add_subdirectory(src)

# Unit Test Directory
add_subdirectory(test)
