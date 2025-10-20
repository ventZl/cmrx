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
    if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/arch/${CMRX_ARCH}/platform/${CMRX_PLATFORM}/CMRX.cmake)
        include(${CMAKE_CURRENT_LIST_DIR}/arch/${CMRX_ARCH}/platform/${CMRX_PLATFORM}/CMRX.cmake)
    endif()
endif()

## Add firmware application definition
# This function is a wrapper around add_library(), which will augment the library to act 
# as a CMRX application.
function(add_application NAME)
	add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${ARGN})
	set_property(TARGET ${NAME} PROPERTY CMRX_IS_APPLICATION 1)
    get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
	target_compile_definitions(${NAME} PRIVATE -D APPLICATION_NAME=${NAME})
    add_custom_command(TARGET ${NAME} POST_BUILD
        COMMAND ${CMRX_ROOT_DIR}/ld/checkapp.sh 
        ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:${NAME}>
        ${NAME}_instance
        COMMENT "Checking application soundness"
    )
endfunction()


