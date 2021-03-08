set(ipc_SRCS mutex.c rpc.c)
set(stdlib_SRCS sysenter.c timer.c)

add_library(ipc STATIC ${ipc_SRCS})

add_library(stdlib STATIC ${stdlib_SRCS})
