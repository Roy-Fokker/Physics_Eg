# Compile HLSL files for given target.

# Function: target_shader_sources
# Usage: target_shader_sources(<target> <PRIVATE|PUBLIC> [<file> <shader profile> ...])
function(target_shader_sources target_project)
    # Find the shader compiler
    find_program(DXC dxc DOC "DirectX Shader Compiler[dxc]")
    find_program(FXC fxc DOC "DirectX Shader Compiler[fxc]")
    if (("${DXC}" STREQUAL "DXC-NOTFOUND") OR (("${FXC}" STREQUAL "FXC-NOTFOUND")))
        message(FATAL_ERROR "[Error]: DirectX Shader Compiler not found")
    endif()
    message(STATUS "[Info]: Found DirectX Shader Compiler")

    # Parse the input
    cmake_parse_arguments(TARGET_SHADER_SOURCES "" "" "PRIVATE;PUBLIC" ${ARGN})

    set(DXC_FLAGS
        -nologo    # don't display copyright info
        -E main    # entry point is always main()
    )

    set(FXC_FLAGS
        /nologo    # don't display copyright info
        /E main    # entry point is always main()
    )

    if(CONFIG:DEBUG)
        set(DXC_FLAGS
            ${dxc_flags}
            -Zi        # include debugging info
            -Od        # disable optimizations
        )
        set(FXC_FLAGS
            ${fxc_flags}
            /Zi        # include debugging info
            /Od        # disable optimizations
        )
    endif()
    
    set(IS_FILE ON)     # a hacky means to alternate between
    set(SHDR_SOURCES)    # source file name  and  
    set(SHDR_PROFILES)   # profile to use when compiling
    set(SHDR_OUTPUTS)    # output file list
    # Loop through the <file profile> pairs
    foreach(file_profile ${TARGET_SHADER_SOURCES_PRIVATE})
        # assume File is 1st param
        if(IS_FILE)
            # Get absolute path of this file and append to Sources list
            get_filename_component(file_abs ${file_profile} ABSOLUTE)
            list(APPEND SHDR_SOURCES ${file_abs})

            # Get file name and append .cso extension, and add to Outputs List
            # Put the output file in same dir as executable.
            get_filename_component(file_we ${file_profile} NAME_WE)
            list(APPEND SHDR_OUTPUTS ${EXECUTABLE_OUTPUT_PATH}/${file_we}.cso)

            # Flip flag for next item
            set(IS_FILE OFF)
        # profile is 2nd param
        else()
            # Append the profile to Profiles List
            list(APPEND SHDR_PROFILES ${file_profile})
            set(IS_FILE ON)
        endif()
    endforeach()

    # Length of Source, Profile and Output Lists should match
    foreach(src prof out IN ZIP_LISTS SHDR_SOURCES SHDR_PROFILES SHDR_OUTPUTS)
        get_filename_component(file ${src} NAME)
        if(${prof} MATCHES "[vpcgdh]+s_[6]+_[0-9]")
            # Call DXC and pass the parameters src, prof and out along with dxc_flags
            add_custom_command(
                TARGET ${target_project} POST_BUILD
                COMMAND ${DXC} ${DXC_FLAGS} -T ${prof} -Fo ${out} ${src}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Building Shader ${file}"
                VERBATIM
            )
        else()
            # Call FXC and pass the parameters src, prof and out along with dxc_flags
            add_custom_command(
                TARGET ${target_project} POST_BUILD
                COMMAND ${FXC} ${FXC_FLAGS} /T ${prof} /Fo ${out} ${src}
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                COMMENT "Building Shader ${file}"
                VERBATIM
            )
        endif()
    endforeach()
    
endfunction()