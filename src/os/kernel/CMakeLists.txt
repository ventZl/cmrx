# otherwise r7 is used by gcc

add_definitions(-fomit-frame-pointer)

if (NOT TESTING)
    set(os_SRCS isr.c rpc.c sched.c signal.c timer.c)
else()
	set(os_SRCS sched.c timer.c)
endif()

add_library(os STATIC EXCLUDE_FROM_ALL ${os_SRCS})

target_link_libraries(os PUBLIC cmrx_arch)

if (TESTING)
	set(test_kernel_SRCS tests/test_sched.c)
	add_executable(test_kernel ${test_kernel_SRCS})
	target_link_libraries(test_kernel os ctest)
	#	target_compile_definitions(os PRIVATE -Dstatic=)

	add_test(NAME test_kernel COMMAND test_kernel)
endif()
