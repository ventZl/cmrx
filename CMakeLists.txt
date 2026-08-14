# We do add sources from within foreign directories, so we want the paths
# absolute.
# We also add dependencies to targets defined elsewhere.
# If any of options defined herein already exists as a variable, then do nothing;
# This allows users to configure the kernel in their CMakeLists.txt
cmake_minimum_required(VERSION 3.15)
cmake_policy(SET CMP0079 NEW)
cmake_policy(SET CMP0076 NEW)
cmake_policy(SET CMP0077 NEW)
set(CMRX_VERSION 0.1.3)
set(CMRX_DOWNLOAD_REPOSITORY git+https://github.com/ventZl/cmrx)
set(CMRX_ORGANIZATION "CMRX Microkernel RTOS Project")
set(CMRX_CONTACT_EMAIL "info@cmrxrtos.org")
set(CMRX_LICENSE_NAME "MIT")
set(CMRX_SPDXID SPDXRef-CMRX-RTOS)

list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
project(cmrx-rtos VERSION ${CMRX_VERSION} LANGUAGES C ASM)
message(DEBUG "CMRX root dir: ${CMAKE_CURRENT_SOURCE_DIR}")
set_property(GLOBAL PROPERTY CMRX_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include(cmake/CPM.cmake)
include(cmake/options.cmake)
include(cmake/unit_test_build.cmake)
include(cmake/clang-tidy.cmake)
include(cmake/testing.cmake)

include_directories(include)
include_directories(${CMAKE_BINARY_DIR})
add_definitions(-Wall -Wextra)
set(CMAKE_C_STANDARD 11)

if (NOT UNIT_TESTING_BUILD)
    # This is a normal target build
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
    if (COMMAND sbom_add_package)
        sbom_add_package(CMRX
            VERSION ${CMRX_VERSION}
            SUPPLIER ORGANIZATION "${CMRX_ORGANIZATION}" EMAIL "${CMRX_CONTACT_EMAIL}"
            SPDXID ${CMRX_SPDXID}
            LICENSE "${CMRX_LICENSE_NAME}"
            DECLARED "${CMRX_LICENSE_NAME}"
            PURPOSE "OPERATING-SYSTEM"
            DOWNLOAD ${CMRX_DOWNLOAD_REPOSITORY}
            )
    endif()
    if (COMMAND sbom_add)
        sbom_add(
            PACKAGE CMRX
            DOWNLOAD_LOCATION ${CMRX_DOWNLOAD_REPOSITORY}
            LICENSE "${CMRX_LICENSE_NAME}"
            SPDXID ${CMRX_SPDXID}
            SUPPLIER "Organization: ${CMRX_ORGANIZATION} (${CMRX_CONTACT_EMAIL})"
            VERSION ${CMRX_VERSION}
            )
    endif()
else()
    CPMAddPackage(
        NAME ctest
        GIT_REPOSITORY "https://github.com/bvdberg/ctest"
        GIT_TAG master
        DOWNLOAD_ONLY YES
        )
    # Parent CMake will set these, get rid of them
    unset(ENV{CC})
    unset(ENV{CXX})
    # This IS the root source directory of the current build and we have
    # -DTESTING=1 passed on the CMake commandline. Either this is a nested run
    # of CMake or CMRX source tree is used to run unit tests.
    cmake_minimum_required(VERSION 3.18)
    project(CMRX_unit_tests LANGUAGES C)

    cmake_policy(SET CMP0079 NEW)
    cmake_policy(SET CMP0076 NEW)

    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/os/arch/testing)

    add_definitions(-DUNIT_TESTING=1)
    add_definitions(-fpack-struct)
    enable_testing()
    set(CMAKE_C_FLAGS -ggdb3)

    set(HAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src/os/arch/testing)

endif()

configure_file(conf/kernel.h ${CMAKE_BINARY_DIR}/conf/kernel.h)
add_subdirectory(src)
if (NOT UNIT_TESTING_BUILD)
    # Testsuite contains HIL tests. This is only relevant
    # in native build.
    add_subdirectory(testsuite)
endif()

if (BUILD_TESTING)
    add_subdirectory(testing)
endif()

apply_clang_tidy(os)
generate_coverage()
