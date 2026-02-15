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
    os_txn.c
    os_notify_wait_object.c
    os_notify_futex.c
    os_shutdown.c
    stubs.c
    test_traits.c
    )


add_executable(test_kernel ${test_kernel_SRCS})
target_link_libraries(test_kernel os ctest)

# The following tests are only supported if SMP is enabled
if (CMRX_ARCH_SMP_SUPPORTED)
    target_sources(test_kernel PRIVATE 
        os_thread_migrate.c
        background_checker.c
        smp_atomic_ops.c
        )
endif()

add_test(NAME test_kernel COMMAND test_kernel)

set(test_algo_SRCS
    algo_data.c
    algo_binary_search.c
    algo_insert_sort.c
    algo_hash_search.c
    )

add_executable(test_algo ${test_algo_SRCS})
target_link_libraries(test_algo ctest)

add_test(NAME test_algo COMMAND test_algo)

get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
set(riscv_hal_backend_test_SRCS
    riscv_hal_backend.c
    riscv_hal_backend_fake_csr.c
    ${CMRX_ROOT_DIR}/src/os/arch/riscv/hal/riscv_hal_backend.c
    )

add_executable(test_riscv_hal_backend ${riscv_hal_backend_test_SRCS})
target_include_directories(test_riscv_hal_backend PRIVATE ${CMRX_ROOT_DIR}/include)
target_link_libraries(test_riscv_hal_backend ctest)
add_test(NAME test_riscv_hal_backend COMMAND test_riscv_hal_backend)

