# Find Depedencies
find_package(fmt REQUIRED)

# Executable to build
add_executable(physics_eg)

# Source for 'physics_eg' executable
target_sources(physics_eg
    PRIVATE
        main.cpp)

# Link with libraries
target_link_libraries(physics_eg
    PRIVATE
        project_configuration
        fmt::fmt)