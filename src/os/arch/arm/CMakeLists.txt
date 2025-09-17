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
