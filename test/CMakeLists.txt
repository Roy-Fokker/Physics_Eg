# Find Depedencies
find_package(Catch2 CONFIG REQUIRED)

# Executable to build
add_executable(physics_eg_tests)

# Source for 'physics_eg_tests' executable
target_sources(physics_eg_tests
    PRIVATE
        test.cpp)

# Link with libraries
target_link_libraries(physics_eg_tests
    PRIVATE
        project_configuration
        Catch2::Catch2)