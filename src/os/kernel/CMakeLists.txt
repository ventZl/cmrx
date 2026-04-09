# otherwise r7 is used by gcc

add_definitions(-fomit-frame-pointer)

if (UNIT_TESTING_BUILD)
    add_definitions(-Dstatic=)
endif()

set(os_SRCS 
    algo.c
    access.c
    context.c
    isr.c
    notify.c
    sched.c 
    signal.c 
    syscall.c 
    timer.c
    txn.c
    rpc.c)

# Add tracing source file if tracing is enabled
if (CMRX_KERNEL_TRACING)
    list(APPEND os_SRCS trace.c)
endif()

add_library(os STATIC EXCLUDE_FROM_ALL ${os_SRCS})
get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
target_include_directories(os INTERFACE ${CMRX_ROOT_DIR}/include ${CMAKE_BINARY_DIR} ${HAL_PATH})

# Just an alias
add_library(cmrx ALIAS os)
if (UNIT_TESTING_BUILD)
    apply_gcov(os)
    add_subdirectory(tests)
endif()
