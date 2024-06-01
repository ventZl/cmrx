add_compile_options(-Wno-unused-parameter)

set(test_kernel_SRCS 
    os_stack_alloc_dispose.c 
    os_get_next_thread.c
    os_start.c
    os_sched_yield.c
    os_thread_kill.c
    os_thread_stop.c
    os_thread_continue.c
    os_setpriority.c
    os_usleep.c
    os_setitimer.c
    rpc_stack_push.c
    rpc_stack_pop.c
    os_sched_timing_callback.c
    )
add_executable(test_kernel ${test_kernel_SRCS})
target_link_libraries(test_kernel os ctest)

add_test(NAME test_kernel COMMAND test_kernel)

