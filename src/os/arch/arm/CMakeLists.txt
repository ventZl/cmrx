set(cmrx_arch_SRCS static.c mpu.c pendsv.c rpc.c sanitize.c sched.c signal.c syscall.c)
add_library(cmrx_arch STATIC ${cmrx_arch_SRCS})
target_link_libraries(cmrx_arch cmsis_core_lib)
