set(test_kernel_SRCS 
    os_stack_alloc_dispose.c 
    os_get_next_thread.c
    os_start.c
    )
add_executable(test_kernel ${test_kernel_SRCS})
target_link_libraries(test_kernel os ctest)

add_test(NAME test_kernel COMMAND test_kernel)

