# Module automating support for PicoSDK-based projects for RP2xxx family of microcontrollers

# RP2350 RISC-V (Hazard3) core: bypass the CMSIS/ARM detection below entirely. PICO_PLATFORM
# is the signal the pico-sdk-riscv quirk itself already keys on, and is guaranteed to be set
# well before this file is ever reached (Pico SDK reads it during pico_sdk_import, which runs
# before include(CMRX)).
if (PICO_PLATFORM STREQUAL "rp2350-riscv")
    set_property(GLOBAL PROPERTY CMRX_ARCH riscv)
    set_property(GLOBAL PROPERTY CMRX_HAL hal)
    return()
endif()

string(TOLOWER "${CMRX_DEVICE}" PICO_DEVICE)
string(TOUPPER "${CMRX_DEVICE}" DEVICE)

# Pico-SDK maintains current CPU frequency in SystemCoreClock
set(CMRX_SYSTICK_USE_SYSTEMCORECLOCK ON)

# Pico-SDK generates map files in format <binary>.elf.map
set(CMRX_MAP_FILE_WITH_EXTENSION ON)

function(__cmrx_rp2xxx_get_patch PICO_SDK_VERSION OUTPUT)
    # versions 1.0.0 and 1.0.1
    if ("${PICO_SDK_VERSION}" VERSION_LESS "1.1.0")
        set(${OUTPUT} "${CMAKE_CURRENT_LIST_DIR}/rp2/linker-1.0.patch" PARENT_SCOPE)
        return()
    endif()

    # versions 1.1.0 - 1.5.0
    if ("${PICO_SDK_VERSION}" VERSION_LESS "1.5.1")
        set(${OUTPUT} "${CMAKE_CURRENT_LIST_DIR}/rp2/linker-1.1.patch" PARENT_SCOPE)
        return()
    endif()

    # versions 1.5.1 only
    if ("${PICO_SDK_VERSION}" VERSION_LESS "2.0.0")
        set(${OUTPUT} "${CMAKE_CURRENT_LIST_DIR}/rp2/linker-1.5.1.patch" PARENT_SCOPE)
        return()
    endif()

    # versions 2.0.0 - 2.2.0 only
    if ("${PICO_SDK_VERSION}" VERSION_LESS "2.3.0")
        set(${OUTPUT} "${CMAKE_CURRENT_LIST_DIR}/rp2/linker-2.0.patch" PARENT_SCOPE)
        return()
    endif()

    # versions 2.3.0 and newer
    set(${OUTPUT} "${CMAKE_CURRENT_LIST_DIR}/rp2/linker-2.3.patch" PARENT_SCOPE)
endfunction()


# Patch linker script file to include properly aligned symbols
# which denote start and end of RAM-deployed code. Pico-SDK does
# that so some routines are in fast RAM instead of slow QSPI FLASH.
function(__cmrx_patch_linker_script FILE OUTFILE)
    __cmrx_rp2xxx_get_patch(${PICO_SDK_VERSION_STRING} __PICO_LINKER_PATCH_FILE)
    if (NOT EXISTS ${__PICO_LINKER_PATCH_FILE})
        message(FATAL_ERROR "Unable to determine linker script patch file. Unsupported Pico-SDK version")
    endif()
    execute_process(
        COMMAND git apply -p1 ${__PICO_LINKER_PATCH_FILE}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        COMMAND_ERROR_IS_FATAL ANY)
    set(CMSIS_LINKER_FILE "${FILE}" PARENT_SCOPE)
endfunction()

if (NOT DEFINED PICO_SDK_PATH)
    message(FATAL_ERROR "Pico SDK path not defined! Set variable `PICO_SDK_PATH` to point to Pico SDK location.")
endif()

set(PICO_LINKER_FILE ${CMAKE_BINARY_DIR}/memmap_default.ld)

if (DEFINED CMRX_LINKER_FILE)
    if (NOT EXISTS "${CMRX_LINKER_FILE}")
        message(FATAL_ERROR "Specified linker file doesn't exist: ${CMRX_LINKER_FILE}")
    endif()
    file(COPY_FILE "${CMRX_LINKER_FILE}" "${PICO_LINKER_FILE}")
else()
    if ("${PICO_SDK_VERSION_STRING}" VERSION_GREATER_EQUAL "2.3.0")
        message(STATUS "Generating default linker script")
        execute_process(
            COMMAND ${PYTHON_EXE} ${PICO_SDK_PATH}/tools/memmap_include_tree.py
                -o ${PICO_LINKER_FILE} ${PICO_SDK_PATH}/src/${PICO_DEVICE}/pico_platform/memmap_default.ld
            WORKING_DIRECTORY ${PICO_SDK_PATH})
    else()
        if ("${PICO_SDK_VERSION_STRING}" VERSION_GREATER_EQUAL "2.0.0")
            set(RAW_LINKER_FILE ${PICO_SDK_PATH}/src/rp2_common/pico_crt0/${PICO_DEVICE}/memmap_default.ld)
        else()
            set(RAW_LINKER_FILE ${PICO_SDK_PATH}/src/rp2_common/pico_standard_link/memmap_default.ld)
        endif()
        message(STATUS "Using existing linker script: ${RAW_LINKER_FILE}")
        file(COPY_FILE "${RAW_LINKER_FILE}" "${PICO_LINKER_FILE}")
    endif()

    if (NOT EXISTS "${PICO_LINKER_FILE}")
        # In the past, PicoSDK changed the location of linker files at least once
        message(FATAL_ERROR "Unable to find Pico SDK linker file for ${DEVICE}! Check Pico SDK installation and compatibility!")
    endif()
endif()

file(STRINGS "${PICO_LINKER_FILE}" __CMRX_LINKERFILE_PATCHES REGEX "__rp2xxx_platform_custom")
if ("${__CMRX_LINKERFILE_PATCHES}" STREQUAL "")
    __cmrx_patch_linker_script(${PICO_LINKER_FILE} ${CMAKE_BINARY_DIR}/memmap_default.ld)
endif()

set(CMSIS_ROOT ${PICO_SDK_PATH}/src/rp2_common/cmsis/stub/CMSIS)
if (NOT EXISTS "${CMSIS_ROOT}")
    message(FATAL_ERROR "Unable to find Pico SDK CMSIS headers! Check Pico SDK installation and compatibility!")
endif()

include(${CMAKE_CURRENT_LIST_DIR}/../FindCMSIS.cmake)

set_property(GLOBAL PROPERTY CMRX_ARCH arm)
set_property(GLOBAL PROPERTY CMRX_HAL cmsis)

if ("${PICO_DEVICE}" STREQUAL "rp2040")
    set(CMRX_PLATFORM "rp20xx")
endif()

