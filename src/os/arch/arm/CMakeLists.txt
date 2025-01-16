set(cmrx_arm_SRCS 
    cortex.c
    static.c 
    mpu.c
    sanitize.c 
    sched.c 
    signal.c 
    rpc.c 
)

target_link_libraries(os PUBLIC cmsis_headers)
target_sources(os PRIVATE ${cmrx_arm_SRCS})
target_compile_options(os PRIVATE -mgeneral-regs-only)

if (CMRX_KERNEL_TRACING)
    include(FetchContent)

    FetchContent_Declare(
        libtrace
        GIT_REPOSITORY https://github.com/orbcode/libtrace.git
        GIT_TAG v0.1.0
    )
    FetchContent_MakeAvailable(libtrace)

    target_link_libraries(os PUBLIC Orbcode::Trace)

    target_sources(os PRIVATE trace.c)
endif()
