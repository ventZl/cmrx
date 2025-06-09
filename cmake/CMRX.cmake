cmake_minimum_required(VERSION 3.18)

find_program(PYTHON_EXE NAMES python3 python REQUIRED DOC "Python 3 executable")

if ("${CMRX_ARCH}" STREQUAL "")
    message(FATAL_ERROR "CMRX_ARCH not defined! Please define target architecture to be used!")
endif()

if ("${CMRX_HAL}" STREQUAL "")
    message(FATAL_ERROR "CMRX_HAL not defined! Please define target HAL to be used!")
endif()

option(SW_TESTING_BUILD "Enabled hosted build. This can be used to build hosted unit tests." FALSE)
option(CMRX_SKIP_LINKER_FILE_USE "Skips automatic use of generated linker file by targets. Developer becomes responsible for the use of correct linker file" FALSE)
option(CMRX_STDLIB_USE_CMSIS_CORE "Forces CMRX standard library to link against cmsis_core library. This is needed with some HALs to successfully link the project" FALSE)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if (SW_TESTING_BUILD)
    include(${CMAKE_CURRENT_LIST_DIR}/arch/testing/CMRX.cmake)
    # Override arch name to "testing"
    set(CMRX_ARCH testing)
    include(CTest)
else()
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/arch/${CMRX_ARCH}/${CMRX_HAL}/CMRX.cmake)
        include(${CMAKE_CURRENT_LIST_DIR}/arch/${CMRX_ARCH}/${CMRX_HAL}/CMRX.cmake)
    endif()
endif()



