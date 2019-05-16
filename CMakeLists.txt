cmake_minimum_required(VERSION 3.1...3.14)

project(VolumeDataSuite VERSION 1.0.0
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

add_subdirectory(external)
add_subdirectory(src)

