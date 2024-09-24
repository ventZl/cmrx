set(stdlib_SRCS rpc.c signal.c timer.c thread.c arch/${CMRX_ARCH}/mutex.c)

add_library(stdlib STATIC ${stdlib_SRCS})
target_include_directories(stdlib PUBLIC ${HAL_PATH})
get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
target_include_directories(stdlib INTERFACE ${CMRX_ROOT_DIR}/include ${CMAKE_BINARY_DIR})

# Let arch to alter the stdlib target if it wants to do so
# It is not an error if architecture does not provide this file.
include(${CMAKE_CURRENT_SOURCE_DIR}/arch/${CMRX_ARCH}/arch.cmake OPTIONAL)
