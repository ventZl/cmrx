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
target_compile_options(os PRIVATE -mgeneral-regs-only)
