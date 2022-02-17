set(os_shim_device_SRCS systick.c)
add_library(os_shim_device STATIC EXCLUDE_FROM_ALL ${os_shim_device_SRCS})