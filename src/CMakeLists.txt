# Find Depedencies
find_package(fmt REQUIRED)
find_package(cppitertools REQUIRED)
find_package(imgui REQUIRED)

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
        os/helper.cpp
        os/helper.h
        gfx/renderer.cpp
        gfx/renderer.h
        gfx/direct3d11.cpp
        gfx/direct3d11.h
        gfx/renderpass.cpp
        gfx/renderpass.h
        gfx/pipeline.cpp
        gfx/pipeline.h
        gfx/gpu_data.cpp
        gfx/gpu_data.h
        gfx/gui.cpp
        gfx/gui.h
        sim/simulation.cpp
        sim/simulation.h
        sim/sim_data.cpp
        sim/sim_data.h)

# Use Precompiled headers for std/os stuff
target_precompile_headers(physics_eg
    PRIVATE
        pch.h)

# Link with libraries
target_link_libraries(physics_eg
    PRIVATE
        project_configuration
        fmt::fmt
        imgui::imgui)

# Build the shaders
target_shader_sources(physics_eg
    PRIVATE
        shader/vs.hlsl vs_5_0
        shader/ps.hlsl ps_5_0)

set_application_manifest(physics_eg manifest.xml)