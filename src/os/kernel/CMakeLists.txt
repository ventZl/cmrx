# otherwise r7 is used by gcc

add_definitions(-fomit-frame-pointer)

if (UNIT_TESTING_BUILD)
    add_definitions(-Dstatic=)
endif()

set(os_SRCS isr.c sched.c signal.c syscall.c timer.c rpc.c)

add_library(os STATIC EXCLUDE_FROM_ALL ${os_SRCS})

# Just an alias
add_library(cmrx ALIAS os)
if (UNIT_TESTING_BUILD)
    add_subdirectory(tests)
endif()
