set(ipc_SRCS mutex.c rpc.c signal.c)
set(stdlib_SRCS sysenter.c timer.c)

add_library(ipc STATIC EXCLUDE_FROM_ALL ${ipc_SRCS})

add_library(stdlib STATIC EXCLUDE_FROM_ALL ${stdlib_SRCS})
