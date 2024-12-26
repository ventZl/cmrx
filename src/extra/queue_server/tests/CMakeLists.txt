add_compile_options(-Wno-unused-parameter)

set(queue_server_test_SRCS
    test_queue_server.c
)

add_executable(queue_server_test ${queue_server_test_SRCS})
target_link_libraries(queue_server_test queue ctest)

add_test(NAME queue_server_test COMMAND queue_server_test)
