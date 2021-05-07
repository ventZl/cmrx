# otherwise r7 is used by gcc

add_definitions(-fomit-frame-pointer)

set(os_SRCS mpu.c pendsv.c rpc.c sanitize.c sched.c signal.c syscall.c timer.c isr.c)

add_library(os OBJECT EXCLUDE_FROM_ALL ${os_SRCS})

if (TESTING)
	set(test_kernel_SRCS tests/test_sched.c)
	add_executable(test_kernel ${test_kernel_SRCS})
	target_link_libraries(test_kernel ctest cmrx_testing)

	add_test(NAME test_kernel COMMAND test_kernel)
endif()
