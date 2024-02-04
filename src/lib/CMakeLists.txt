add_subdirectory(arch/${CMRX_ARCH})

set(stdlib_SRCS rpc.c signal.c timer.c thread.c) # arch/${CMRX_ARCH}/mutex.c)

add_library(stdlib STATIC ${stdlib_SRCS})
target_link_libraries(stdlib stdlib_arch)
#target_include_directories(stdlib PUBLIC ${HAL_PATH})

