add_application(test_init init.c)
target_link_libraries(test_init ${LIBOPENCM3})

set(os_start_SRCS main.c debug.c)
add_firmware(os_start ${os_start_SRCS})
target_link_libraries(os_start os stdlib nosys test_init ${LIBOPENCM3})

add_hw_test(os_start)
