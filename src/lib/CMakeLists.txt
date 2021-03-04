set(ipc_SRCS mutex.c)
set(stdlib_SRCS sysenter.c)

add_library(ipc STATIC ${ipc_SRCS})

add_library(stdlib STATIC ${stdlib_SRCS})
