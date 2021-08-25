cmake_minimum_required(VERSION 3.1...3.14)

project(VDS VERSION 1.0.0
                  DESCRIPTION "Volume Data Suite"
                  LANGUAGES CXX)

configure_file (
    "${PROJECT_SOURCE_DIR}/include/version.h.in"
    "${PROJECT_BINARY_DIR}/include/version.h"
)

# Get Git submodule dependencies
find_package(Git QUIET)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
    execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init --recursive
                    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                    RESULT_VARIABLE GIT_SUBMOD_RESULT)
    if(NOT GIT_SUBMOD_RESULT EQUAL "0")
        message(FATAL_ERROR "git submodule update --init --recursive failed with ${GIT_SUBMOD_RESULT}, 
        please checkout submodules manually with \"git submodule update --init --recursive\"")
    endif()
endif()


# This function replicates the directory hierarchy of a target, so that the layout of the files in IDEs match
function(replicate_directory_structure target_name)
    # Add any files that are not in the source list to a separate group 
    source_group("other files" REGULAR_EXPRESSION ".")
    get_target_property(target_sources "${target_name}" SOURCES)
    
    foreach(file_name IN LISTS target_sources)
        get_filename_component(directory_name "${file_name}" PATH)
        # Group names use '\\' instead of '/'
        string(REPLACE "/" "\\" group_name "${directory_name}")
        source_group("${group_name}" FILES "${file_name}")
    endforeach()
endfunction()

add_subdirectory(external)
add_subdirectory(src)


# Set VDS project as default start up project in Visual Studio
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT VDS)


