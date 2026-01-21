set(cmrx_linux_SRCS
    posix.c
    clock.c
    static.c
    thread.c
    interrupt.c
    relay.c
    )

if (APPLE)
    add_library(c11 STATIC threads.c)
    target_include_directories(c11 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/c11)
    target_link_libraries(c11 pthread)

    target_link_libraries(os PUBLIC c11)
endif()

target_sources(os PRIVATE ${cmrx_linux_SRCS})
