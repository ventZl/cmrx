set(os_shim_SRCS static.c)

add_library(os_shim STATIC EXCLUDE_FROM_ALL ${os_shim_SRCS})

add_library(ctest STATIC EXCLUDE_FROM_ALL ctest_main.c)
