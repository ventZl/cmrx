cmake_minimum_required(VERSION 3.18)


find_program(PYTHON_EXE NAMES python3 python python3.exe python.exe py REQUIRED DOC "Python 3 executable")

if (DEFINED CMRX_DEVICE)
    set(DEVICE "${CMRX_DEVICE}")
    string(LENGTH "${CMRX_DEVICE}" _DEV_LEN)
    set(_DEV_PREFIX_LEN 2)
    while(_DEV_PREFIX_LEN LESS _DEV_LEN)
        string(SUBSTRING "${CMRX_DEVICE}" 0 ${_DEV_PREFIX_LEN} _DEV_PREFIX)
        string(TOLOWER "${_DEV_PREFIX}" _DEV_PREFIX)
        set(PLATFORM_SCRIPT "${CMAKE_CURRENT_LIST_DIR}/platform/${_DEV_PREFIX}.cmake")
        if (EXISTS ${PLATFORM_SCRIPT})
            include(${PLATFORM_SCRIPT})
            break()
        endif()
        math(EXPR _DEV_PREFIX_LEN "${_DEV_PREFIX_LEN} + 1")
    endwhile()
endif()

if ("${CMRX_ARCH}" STREQUAL "")
    get_property(CMRX_ARCH GLOBAL PROPERTY CMRX_ARCH)
    if (NOT DEFINED CMRX_ARCH)
        message(FATAL_ERROR "CMRX_ARCH not defined! Please define target architecture to be used!")
    endif()
endif()

if ("${CMRX_HAL}" STREQUAL "")
    get_property(CMRX_HAL GLOBAL PROPERTY CMRX_HAL)
    if (NOT DEFINED CMRX_HAL)
        message(FATAL_ERROR "CMRX_HAL not defined! Please define target HAL to be used!")
    endif()
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



