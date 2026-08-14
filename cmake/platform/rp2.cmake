# Module automating support for PicoSDK-based projects for RP2xxx family of microcontrollers
string(TOLOWER "${CMRX_DEVICE}" PICO_DEVICE)
string(TOUPPER "${CMRX_DEVICE}" DEVICE)

if (NOT DEFINED PICO_SDK_PATH)
    message(FATAL_ERROR "Pico SDK path not defined! Set variable `PICO_SDK_PATH` to point to Pico SDK location.")
endif()

if (DEFINED CMRX_LINKER_FILE)
    if (NOT EXISTS "${CMRX_LINKER_FILE}")
        message(FATAL_ERROR "Specified linker file doesn't exist: ${CMRX_LINKER_FILE}")
    endif()
else()
    set(CMSIS_LINKER_FILE "${CMRX_LINKER_FILE}")
    set(CMSIS_LINKER_FILE ${PICO_SDK_PATH}/rp2_common/pico_crt0/${PICO_DEVICE}/memmap_default.ld)
    if (NOT EXISTS "{CMSIS_LINKER_FILE}")
        # In the past, PicoSDK changed the location of linker files at least once
        message(FATAL_ERROR "Unable to find Pico SDK linker file for ${DEVICE}! Check Pico SDK installation and compatibility!")
    endif()
endif()

set(CMSIS_ROOT ${PICO_SDK_PATH}/src/rp2_common/cmsis/stub/CMSIS)
if (NOT EXISTS "${CMSIS_ROOT}")
    message(FATAL_ERROR "Unable to find Pico SDK CMSIS headers! Check Pico SDK installation and compatibility!")
endif()

include(../FindCMSIS.cmake)

set_property(GLOBAL PROPERTY CMRX_ARCH arm)
set_property(GLOBAL PROPERTY CMRX_HAL cmsis)

