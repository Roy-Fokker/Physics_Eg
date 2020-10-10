# Find Depedencies
find_package(fmt REQUIRED)
find_package(cppitertools REQUIRED)

# Executable to build
add_executable(physics_eg)

# Source for 'physics_eg' executable
target_sources(physics_eg
    PRIVATE
        main.cpp
        os/window.cpp
        os/window.h
        os/clock.cpp
        os/clock.h
        gfx/renderer.cpp
        gfx/renderer.h
        gfx/direct3d11.cpp
        gfx/direct3d11.h
        gfx/renderpass.cpp
        gfx/renderpass.h)

# Use Precompiled headers for std/os stuff
target_precompile_headers(physics_eg
    PRIVATE
        pch.h)

# Link with libraries
target_link_libraries(physics_eg
    PRIVATE
        project_configuration
        fmt::fmt)

# Build the shaders
target_shader_sources(physics_eg
    PRIVATE
        shader/vs.hlsl vs_5_1
        shader/ps.hlsl ps_5_1)