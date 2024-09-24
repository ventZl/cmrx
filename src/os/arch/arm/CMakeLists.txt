set(cmrx_arm_SRCS 
    static.c 
    mpu.c 
    pendsv.c 
    sanitize.c 
    sched.c 
    signal.c 
    rpc.c 
    syscall.c
)

target_link_libraries(os PUBLIC cmsis_headers)
target_sources(os PRIVATE ${cmrx_arm_SRCS})
