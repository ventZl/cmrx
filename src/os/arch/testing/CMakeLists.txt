set(cmrx_testing_SRCS
    corelocal.c
    static.c
    mpu.c
    sched.c
    timing_provider.c
    pendsv.c
    )
target_sources(os PRIVATE ${cmrx_testing_SRCS})
target_link_libraries(os PRIVATE ctest)
target_include_directories(os PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
