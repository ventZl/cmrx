set(ipc_SRCS mutex.c rpc.c signal.c)
set(stdlib_SRCS sysenter.c timer.c)
set(pthread_SRCS thread.c)

add_library(ipc STATIC EXCLUDE_FROM_ALL ${ipc_SRCS})

add_library(stdlib STATIC EXCLUDE_FROM_ALL ${stdlib_SRCS})

add_library(pthread STATIC EXCLUDE_FROM_ALL ${pthread_SRCS})