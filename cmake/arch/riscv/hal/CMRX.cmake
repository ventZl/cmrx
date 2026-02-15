# CMRX CMake module for RISC-V HAL
#
# Provides minimal cmake infrastructure for RISC-V targets.
# Note: MPU linker script management is not yet implemented for RISC-V.

message(STATUS "RISC-V HAL CMRX CMake component loaded")

find_program(PYTHON_EXE NAMES python3 python REQUIRED DOC "Python 3 executable")

# Path to the CMRX sections linker script fragment
set(CMRX_RISCV_SECTIONS_LD "${CMAKE_CURRENT_LIST_DIR}/cmrx_sections.ld")

# Stub: add_firmware wraps add_executable for RISC-V
# Full linker script management is not yet implemented
function(add_firmware FW_NAME)
    get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
    if ("${CMRX_ROOT_DIR}" STREQUAL "")
        message(FATAL_ERROR "CMRX source tree directory property not set! Did you forget to add_subdirectory(cmrx)?")
    endif()

    add_executable(${FW_NAME} ${ARGN})
    set_property(TARGET ${FW_NAME} PROPERTY CMRX_IS_FIRMWARE 1)

    # Generate map file for analysis
    target_link_options(${FW_NAME} PUBLIC -Wl,-Map=${FW_NAME}.map)

    # Include the CMRX sections linker script fragment.
    # This defines .cmrx_applications and .cmrx_thread_create sections
    # with __applications_start/__applications_end and
    # __thread_create_start/__thread_create_end boundary symbols.
    target_link_options(${FW_NAME} PUBLIC
        "-T${CMRX_RISCV_SECTIONS_LD}"
    )
endfunction()

# Stub: target_add_applications links applications to firmware
# Full linker script application support is not yet implemented
function(target_add_applications TGT_NAME)
    target_link_libraries(${TGT_NAME} ${ARGN})

    get_target_property(IS_FIRMWARE ${TGT_NAME} CMRX_IS_FIRMWARE)
    if ("${IS_FIRMWARE}" EQUAL "1")
        foreach(LIBRARY ${ARGN})
            get_target_property(IS_APPLICATION ${LIBRARY} CMRX_IS_APPLICATION)
            if ("${IS_APPLICATION}" EQUAL "1")
                message(STATUS "Adding application ${LIBRARY} to firmware ${TGT_NAME}")
            endif()
        endforeach()
    endif()
endfunction()

## Add firmware application definition
# RISC-V implementation mirrors the generic/static-library model used by
# other architectures and marks the target as a CMRX application.
function(add_application NAME)
    get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
    if ("${CMRX_ROOT_DIR}" STREQUAL "")
        message(FATAL_ERROR "CMRX source tree directory property not set! Did you forget to add_subdirectory(cmrx)?")
    endif()

    add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${ARGN})
    set_property(TARGET ${NAME} PROPERTY CMRX_IS_APPLICATION 1)
    target_compile_definitions(${NAME} PRIVATE -D APPLICATION_NAME=${NAME})
    target_include_directories(${NAME} PRIVATE
        ${CMRX_ROOT_DIR}/include
        ${CMRX_ROOT_DIR}/src/os/arch/${CMRX_ARCH}/${CMRX_HAL}
        ${CMAKE_BINARY_DIR}
    )
endfunction()
