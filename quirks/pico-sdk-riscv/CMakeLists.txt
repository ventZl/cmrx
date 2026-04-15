# CMRX quirk for Pico SDK RISC-V (RP2350)
#
# Provides:
# - Machine timer ISR with full ExceptionFrame save/restore
# - Exception dispatcher with full ExceptionFrame save/restore
# - ecall exception handler for CMRX syscalls

if(NOT DEFINED PICO_PLATFORM)
    return()
endif()

if(NOT PICO_PLATFORM STREQUAL "rp2350-riscv")
    return()
endif()

message(STATUS "CMRX: Pico SDK RISC-V quirk enabled")

target_sources(os PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}/cmrx_timer_isr.c
    ${CMAKE_CURRENT_LIST_DIR}/cmrx_exception_dispatcher.c
    ${CMAKE_CURRENT_LIST_DIR}/cmrx_ecall_handler.c
)

target_include_directories(os PRIVATE
    ${CMRX_ROOT_DIR}/include
)

target_link_libraries(os PRIVATE
    pico_base_headers
    hardware_regs
)
