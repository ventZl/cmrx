set(queue_lib_SRCS queue.c)
add_library(queue STATIC ${queue_lib_SRCS})

if (NOT UNIT_TESTING_BUILD)
    add_application(queue_server queue_server.c)
    target_link_libraries(queue_server queue)
endif()

if (UNIT_TESTING_BUILD)
    apply_gcov(queue)
    add_subdirectory(tests)
endif()
