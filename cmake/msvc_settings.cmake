# Setup configuration specific to MSVC
function(set_msvc_project_configuration project_name)
    option(MSVC_CXX_17_MODE "Enable C++17" ON)
    option(MSVC_CXX_LATEST  "Enable C++Latest" OFF)
    option(MSVC_WIN32 "Enable Windows Subsystem" OFF)

    set(MSVC_COMPILER_FLAGS
            /W3             # Enable all warnings
            /permissive-    # enable conformance mode
            /Zc:__cplusplus # report C++ version correctly
    )

    set(MSVC_LINKER_FLAGS
        /entry:mainCRTStartup # use int main() as entry point.
    )

    set(MSVC_COMPILE_DEFINITIONS
            UNICODE _UNICODE    # Windows.h use UNICODE
            _SILENCE_CXX17_C_HEADER_DEPRECATION_WARNING # disable C header complaints
            NOMINMAX            # Don't use Windows.h min/max
            WIN32_LEAN_AND_MEAN # Trim it down.
    )

    # if both options are true, only use latest mode is enabled.
    # if neither option is enabled, MSVC will use it's default.
    if(MSVC_CXX_LATEST)
        set(MSVC_COMPILER_FLAGS 
                ${MSVC_COMPILER_FLAGS}
                /std:c++latest  # use C++ Latest features (20?)
        )
    elseif(MSVC_CXX_17_MODE)
        set(MSVC_COMPILER_FLAGS 
                ${MSVC_COMPILER_FLAGS}
                /std:c++17      # use C++ 17 
        )
    endif()

    # If we are building for Windows GUI
    if(MSVC_WIN32)
        set(MSVC_LINKER_FLAGS
                ${MSVC_LINKER_FLAGS}
                /subsystem:Windows
        )
    endif()

    # Only enable if current compiler is MSVC.
    if (MSVC)
        message("Enable MSVC configuraiton.")

        target_compile_options(${project_name}
            INTERFACE
                ${MSVC_COMPILER_FLAGS}
        )

        target_compile_definitions(${project_name}
            INTERFACE
                ${MSVC_COMPILE_DEFINITIONS}
        )

        target_link_options(${project_name}
            INTERFACE
                ${MSVC_LINKER_FLAGS}
        )
    endif()
endfunction(set_msvc_project_configuration project_name)

# Amend application manifest
function(set_application_manifest project_name manifest_file)
    get_filename_component(manifest_file ${manifest_file} ABSOLUTE)
    
    # Only run if we are compiling with MSVC
    if (MSVC)
        # Make sure the CMake version if greater than 3.0
        if (CMAKE_MAJOR_VERSION LESS 3)
            message(WARNING "CMake version 3.0 or newer is required use build variable TARGET_FILE")
        else()
            ADD_CUSTOM_COMMAND(
                TARGET ${project_name} POST_BUILD
                COMMAND "mt.exe" 
                        -nologo
                        -manifest \"${manifest_file}\" 
                        -inputresource:\"$<TARGET_FILE:${project_name}>\"\;\#1 
                        -outputresource:\"$<TARGET_FILE:${project_name}>\"\;\#1
                COMMENT "Adding display aware manifest..." 
            )
        endif()
    ENDIF(MSVC)
endfunction(set_application_manifest project_name manifest_file)