# otherwise r7 is used by gcc

add_definitions(-fomit-frame-pointer)

if (NOT TESTING)
    set(os_SRCS isr.c sched.c signal.c syscall.c timer.c rpc.c)
else()
	set(os_SRCS sched.c timer.c)
endif()

add_library(os STATIC EXCLUDE_FROM_ALL ${os_SRCS})
target_link_libraries(os PRIVATE cmrx_arch)

# The umbrella library that wraps cross-platform and platform-specific portions of kernel 
# This is a rather ugly solution to the problem that older versions of CMake can't natively
# support --start-group and --end-group where it is needed.
# Thus we create an interface library, which has the command specified verbatim.
# This thing is most probably extremely fragile and will break in the moment someone else
# tries to use --end-group with something else.
add_library(cmrx INTERFACE)
add_dependencies(cmrx os cmrx_arch)
target_link_libraries(cmrx INTERFACE "-Wl,--start-group" $<TARGET_FILE:os> $<TARGET_FILE:cmrx_arch> "-Wl,--end-group")
add_custom_command(OUTPUT ${CMRX_CURRENT_BINARY_DIR}/libcmrx.a
    COMMAND ${CMAKE_AR_COMMAND} -)

if (TESTING)
	set(test_kernel_SRCS tests/test_sched.c)
	add_executable(test_kernel ${test_kernel_SRCS})
	target_link_libraries(test_kernel os ctest)
	#	target_compile_definitions(os PRIVATE -Dstatic=)

	add_test(NAME test_kernel COMMAND test_kernel)
endif()
