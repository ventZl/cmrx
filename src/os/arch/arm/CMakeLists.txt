set(cmrx_arm_SRCS 
    cortex.c
    static.c 
    mpu.c
    runtime.c
    sanitize.c 
    sched.c 
    signal.c 
    rpc.c 
)

target_link_libraries(os PUBLIC cmsis_headers)
target_sources(os PRIVATE ${cmrx_arm_SRCS})
if (NOT ${CMAKE_C_COMPILER_ID} STREQUAL "Clang")
   target_compile_options(os PRIVATE -mgeneral-regs-only)
endif()

message(STATUS "Target platform: ${CMRX_PLATFORM}")
if (CMRX_PLATFORM)
    if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${CMRX_PLATFORM}")
        add_subdirectory(${CMRX_PLATFORM})
    else()
        message(FATAL_ERROR "Unable to include platform ${CMRX_PLATFORM} as it doesn't exist!")
    endif()
endif()
