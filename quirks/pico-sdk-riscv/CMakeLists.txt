# CMRX quirk for Pico SDK RISC-V (RP2350)
#
# Provides:
# - Replacement external IRQ handler that calls CMRX context switch safe-point
# - ecall exception handler for CMRX syscalls

if(NOT DEFINED PICO_PLATFORM)
    return()
endif()

if(NOT PICO_PLATFORM STREQUAL "rp2350-riscv")
    return()
endif()

message(STATUS "CMRX: Pico SDK RISC-V quirk enabled")

# Add the CMRX RISC-V handlers to the OS library
target_sources(os PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/crt0_riscv_cmrx.S
    ${CMAKE_CURRENT_LIST_DIR}/cmrx_ecall_handler.c
    ${CMAKE_CURRENT_LIST_DIR}/riscv_rp2350_exception_frame.c
)

# Ensure headers are accessible:
# - CMRX context switch header
# - Pico SDK headers for the assembly file (pico.h, hardware/regs/rvcsr.h)
target_include_directories(os PRIVATE
    ${CMRX_ROOT_DIR}/include
)

# Link against pico_base_headers to get Pico SDK includes
target_link_libraries(os PRIVATE
    pico_base_headers
    hardware_regs
)
