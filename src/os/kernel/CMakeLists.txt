# otherwise r7 is used by gcc
add_definitions(-fomit-frame-pointer)

set(os_SRCS pendsv.c sched.c mpu.c syscall.c rpc.c)

add_library(os OBJECT ${os_SRCS})
