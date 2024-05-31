# otherwise r7 is used by gcc

add_definitions(-fomit-frame-pointer)

if (UNIT_TESTING)
    add_definitions(-Dstatic=)
endif()

if (NOT UNIT_TESTING)
    set(os_SRCS isr.c sched.c signal.c syscall.c timer.c rpc.c)
else()
    set(os_SRCS isr.c sched.c signal.c syscall.c timer.c rpc.c)
	#set(os_SRCS sched.c timer.c)
endif()

add_library(os STATIC EXCLUDE_FROM_ALL ${os_SRCS})

# Just an alias
add_library(cmrx ALIAS os)
if (TESTING)
    add_subdirectory(tests)
endif()
