# Module automating support for CubeMX-based projects for STM32 family of microcontrollers

set_property(GLOBAL PROPERTY CMRX_ARCH arm)
set_property(GLOBAL PROPERTY CMRX_HAL cmsis)
set(DEVICE ${CMRX_DEVICE})
string(TOUPPER "${CMRX_DEVICE}" _CMRX_DEVICE_UPCASE)
add_definitions(-D${_CMRX_DEVICE_UPCASE})
string(SUBSTRING "${CMRX_DEVICE}" 0 7 _STM_FAMILY)
string(TOLOWER "${_STM_FAMILY}" _STM_FAMILY)
string(TOUPPER "${_STM_FAMILY}" _STM_FAMILY_UC)

set(SYSTEM_INCLUDE_FILENAME system_${_STM_FAMILY}xx.h)
set(DEVICE_INCLUDE_FILENAME ${_STM_FAMILY}xx.h)

# Assume default CubeMX CMake project layout
if (NOT EXISTS ${CMAKE_SOURCE_DIR}/Drivers/CMSIS)
    message(FATAL_ERROR "CubeMX CMSIS package not found! STM32 platform support assumes default CubeMX project layout. Reorganize project or use manual HAL integration")
endif()

set(CMSIS_ROOT ${CMAKE_SOURCE_DIR}/Drivers/CMSIS)

if (DEFINED CMRX_LINKER_FILE)
    if (NOT EXISTS "${CMRX_LINKER_FILE}")
        message(FATAL_ERROR "Specified linker file doesn't exist: ${CMRX_LINKER_FILE}")
    endif()
    set(CMSIS_LINKER_FILE "${CMRX_LINKER_FILE}")
else()
    file(GLOB CMSIS_LINKER_FILE LIST_DIRECTORIES false "${CMAKE_SOURCE_DIR}/${_STM_FAMILY}*.ld" "${CMAKE_SOURCE_DIR}/${_STM_FAMILY_UC}*.ld")
    list(LENGTH CMSIS_LINKER_FILE _LINKER_FILES_FOUND)
    if (NOT "${_LINKER_FILES_FOUND}" STREQUAL 1)
        message(FATAL_ERROR "Unable to identify project linker file automatically! Either set variable `CMRX_LINKER_FILE` to contain path to linker file used or place exactly one linker file having pattern ${_STM_FAMILY}*.ld into ${CMAKE_SOURCE_DIR}")
    endif()
endif()
add_definitions(-D${CMRX_DEVICE})

include(${CMAKE_CURRENT_LIST_DIR}/../FindCMSIS.cmake)

function(_create_stm32cube_headers)
    if (NOT TARGET stm32cubemx)
        message(FATAL_ERROR "Target stm32cubemx not defined! Your project is not including subdirectory cmake/stm32cubemx or the generated project is not compatible with this STM32 platform support module!")
    endif()
    get_target_property(CUBEMX_INCLUDE_DIRECTORIES stm32cubemx INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(CUBEMX_COMPILE_DEFINITIONS stm32cubemx INTERFACE_COMPILE_DEFINITIONS)

    add_library(stm32cubemx_headers INTERFACE)
    target_include_directories(stm32cubemx_headers INTERFACE ${CUBEMX_INCLUDE_DIRECTORIES})
    target_compile_definitions(stm32cubemx_headers INTERFACE ${CUBEMX_COMPILE_DEFINITIONS})
    get_property(LINK_OPTS TARGET ${CMAKE_PROJECT_NAME} PROPERTY LINK_OPTIONS)
endfunction()

if (TARGET stm32cubemx)
    _create_stm32cube_headers()
else()
    message(STATUS "Target stm32cubemx not defined. Deferring creation of stm32cubemx_headers.")
    cmake_language(DEFER DIRECTORY ${CMAKE_SOURCE_DIR} CALL _create_stm32cube_headers)
endif()

string(REGEX REPLACE "-T \"[^\"]\+\"" "" CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
message("Linker flags: ${CMAKE_EXE_LINKER_FLAGS}")
