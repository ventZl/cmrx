set(cmrx_testing_SRCS
    corelocal.c
    static.c
    mpu.c
    sched.c
    timing_provider.c
    pendsv.c
    )
target_sources(os PRIVATE ${cmrx_testing_SRCS})

