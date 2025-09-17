cmake_minimum_required(VERSION 3.18)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/../../cmake")

if (NOT CMRX_TARGET_PLATFORM)
    message(FATAL_ERROR "Use -DCMRX_TARGET_PLATFORM to select target platform to run clang-tidy on")
elseif("${CMRX_TARGET_PLATFORM}" STREQUAL "ARMv6M")
    add_compile_options(--target=arm-none-eabi)
    set(CMSIS_ROOT ${CMAKE_SOURCE_DIR}/mock/cmsis)
    set(DEVICE cortex-m0+)
    set(CMSIS_LINKER_FILE ${CMAKE_SOURCE_DIR}/mock/cmsis/cortex-m0+.ld)
    include(FindCMSIS)

    set(CMRX_ARCH arm)
    set(CMRX_HAL cmsis)
elseif("${CMRX_TARGET_PLATFORM}" STREQUAL "ARMv7M")
    add_compile_options(--target=arm-none-eabi)
    set(CMSIS_ROOT ${CMAKE_SOURCE_DIR}/mock/cmsis)
    set(DEVICE cortex-m3)
    set(CMSIS_LINKER_FILE ${CMAKE_SOURCE_DIR}/mock/cmsis/cortex-m3.ld)
    include(FindCMSIS)


    set(CMRX_ARCH arm)
    set(CMRX_HAL cmsis)
elseif("${CMRX_TARGET_PLATFORM}" STREQUAL "ARMv7MF")
    add_compile_options(--target=arm-none-eabi)
    set(CMSIS_ROOT ${CMAKE_SOURCE_DIR}/mock/cmsis)
    set(DEVICE cortex-m4f)
    set(CMSIS_LINKER_FILE ${CMAKE_SOURCE_DIR}/mock/cmsis/cortex-m4f.ld)
    include(FindCMSIS)

    set(CMRX_ARCH arm)
    set(CMRX_HAL cmsis)
endif()

project(cmrx-clang-tidy LANGUAGES ASM C)

include(CMRX)

add_subdirectory(../../ ${CMAKE_BINARY_DIR}/cmrx)
