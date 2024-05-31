set(cmrx_testing_SRCS
    static.c
    mpu.c
    sched.c
    timing_provider.c
    pendsv.c
    )
target_sources(os PRIVATE ${cmrx_testing_SRCS})

