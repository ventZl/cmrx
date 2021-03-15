# otherwise r7 is used by gcc
add_definitions(-fomit-frame-pointer)

set(os_SRCS mpu.c pendsv.c rpc.c sanitize.c sched.c syscall.c timer.c)

add_library(os OBJECT ${os_SRCS})
