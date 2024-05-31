# We do add sources from within foreign directories, so we want the paths
# absolute.
# We also add dependencies to targets defined elsewhere.
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0076 NEW)

message(DEBUG "CMRX root dir: ${CMAKE_CURRENT_SOURCE_DIR}")
set_property(GLOBAL PROPERTY CMRX_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(include)
include_directories(${CMAKE_BINARY_DIR})
add_definitions(-Wall -Wextra)
set(CMAKE_C_STANDARD 11)

if (NOT UNIT_TESTING)
    if (NOT CMRX_ARCH)
        message(FATAL_ERROR "Target architecture not defined!")
    endif()

    if (NOT CMRX_HAL)
        message(FATAL_ERROR "HAL not selected!")
    endif()

    set(HAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include/cmrx/arch/${CMRX_ARCH}/${CMRX_HAL})
    get_filename_component(HAL_PATH ${HAL_PATH} ABSOLUTE)
    message(DEBUG "HAL path: ${HAL_PATH}")
    if (NOT IS_DIRECTORY ${HAL_PATH})
        message(FATAL_ERROR "Architecture ${CMRX_ARCH} does not support HAL ${CMRX_HAL}!")
    endif()

    include_directories(${HAL_PATH})
else()
    if (NOT ("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}"))
        # This is NOT the root source directory of the current build and we have 
        # -DUNIT_TESTING=1 passed on the CMake commandline. Here we will run child CMake 
        # to configure CMRX root tree with 
        file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
        execute_process(COMMAND ${CMAKE_COMMAND} -DUNIT_TESTING=1 ${CMAKE_CURRENT_SOURCE_DIR}
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
        # Target to build all tests
        add_custom_target(build_tests ALL COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR})

        # Target to run all unit tests
        add_custom_target(unit_tests COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} -- test)
    else()
        # This IS the root source directory of the current build and we have
        # -DTESTING=1 passed on the CMake commandline. Either this is a nested run
        # of CMake or CMRX source tree is used to run unit tests.
        cmake_minimum_required(VERSION 3.5)
        project(CMRX_unit_tests LANGUAGES C)

        cmake_policy(SET CMP0079 NEW)
        cmake_policy(SET CMP0076 NEW)

        include_directories(ctest)
        include_directories(include/testing)
        message(STATUS "Configuring CMRX unit testing source tree")

        include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include/cmrx/arch/testing)
        add_definitions(-DUNIT_TESTING=1)
        enable_testing()
        set(CMAKE_C_FLAGS -ggdb3)

    endif()
endif()

configure_file(conf/kernel.h ${CMAKE_BINARY_DIR}/conf/kernel.h)
add_subdirectory(src)
if (NOT UNIT_TESTING)
    # Testsuite contains HIL tests. This is only relevant
    # in native build.
    add_subdirectory(testsuite)
endif()

