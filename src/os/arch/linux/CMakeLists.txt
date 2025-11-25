set(cmrx_linux_SRCS
    linux.c
    clock.c
    static.c
    thread.c
    interrupt.c
    )

target_sources(os PRIVATE ${cmrx_linux_SRCS})
