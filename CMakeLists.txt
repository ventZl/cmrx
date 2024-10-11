# We do add sources from within foreign directories, so we want the paths
# absolute.
# We also add dependencies to targets defined elsewhere.
# If any of options defined herein already exists as a variable, then do nothing;
# This allows users to configure the kernel in their CMakeLists.txt
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0077 NEW)

message(DEBUG "CMRX root dir: ${CMAKE_CURRENT_SOURCE_DIR}")
set_property(GLOBAL PROPERTY CMRX_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include(cmake/options.cmake)
include(cmake/unit_test_build.cmake)

include_directories(include)
include_directories(${CMAKE_BINARY_DIR})
add_definitions(-Wall -Wextra)
set(CMAKE_C_STANDARD 11)

if (NOT UNIT_TESTING_BUILD)
    if (NOT CMRX_ARCH)
        message(FATAL_ERROR "Target architecture not defined!")
    endif()

    if (NOT CMRX_HAL)
        message(FATAL_ERROR "HAL not selected!")
    endif()

    set(HAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/os/arch/${CMRX_ARCH}/${CMRX_HAL})
    get_filename_component(HAL_PATH ${HAL_PATH} ABSOLUTE)
    message(STATUS "HAL path: ${HAL_PATH}")
    if (NOT IS_DIRECTORY ${HAL_PATH})
        message(FATAL_ERROR "Architecture ${CMRX_ARCH} does not support HAL ${CMRX_HAL}!")
    endif()

    include_directories(${HAL_PATH})
else()
    # Parent CMake will set these, get rid of them
    unset(ENV{CC})
    unset(ENV{CXX})
    # This IS the root source directory of the current build and we have
    # -DTESTING=1 passed on the CMake commandline. Either this is a nested run
    # of CMake or CMRX source tree is used to run unit tests.
    cmake_minimum_required(VERSION 3.5)
    project(CMRX_unit_tests LANGUAGES C)

    cmake_policy(SET CMP0079 NEW)
    cmake_policy(SET CMP0076 NEW)

    include_directories(ctest)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/os/arch/testing)

    add_definitions(-DUNIT_TESTING=1)
    add_definitions(-fpack-struct)
    enable_testing()
    set(CMAKE_C_FLAGS -ggdb3)
endif()

configure_file(conf/kernel.h ${CMAKE_BINARY_DIR}/conf/kernel.h)
add_subdirectory(src)
if (NOT UNIT_TESTING_BUILD)
    # Testsuite contains HIL tests. This is only relevant
    # in native build.
    add_subdirectory(testsuite)
endif()

