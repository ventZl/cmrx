# otherwise r7 is used by gcc

add_definitions(-fomit-frame-pointer)

if (UNIT_TESTING_BUILD)
    add_definitions(-Dstatic=)
endif()

set(os_SRCS 
    algo.c
    access.c
    context.c
    isr.c
    notify.c
    sched.c 
    signal.c 
    syscall.c 
    timer.c
    txn.c
    rpc.c)

# Add tracing source file if tracing is enabled
if (CMRX_KERNEL_TRACING)
    list(APPEND os_SRCS trace.c)
endif()

get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)

if (CMRX_ENABLE_LTO)
    add_library(os EXCLUDE_FROM_ALL STATIC ${os_SRCS})
    target_include_directories(os INTERFACE ${CMRX_ROOT_DIR}/include ${CMAKE_BINARY_DIR} ${HAL_PATH})

    add_custom_command(OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/unity.c
        COMMAND ${PYTHON_EXE} ${CMRX_ROOT_DIR}/util/unify.py ${CMAKE_CURRENT_SOURCE_DIR}/unity.c $<TARGET_PROPERTY:os,SOURCES>
        COMMAND_EXPAND_LISTS
        )

    add_library(os_lto STATIC EXCLUDE_FROM_ALL ${CMAKE_CURRENT_SOURCE_DIR}/unity.c)

    target_include_directories(os_lto INTERFACE $<TARGET_PROPERTY:os,INTERFACE_INCLUDE_DIRECTORIES>)
    target_include_directories(os_lto PRIVATE $<TARGET_PROPERTY:os,INCLUDE_DIRECTORIES>)

    target_compile_definitions(os_lto INTERFACE $<TARGET_PROPERTY:os,INTERFACE_COMPILE_DEFINITIONS>)
    target_compile_definitions(os_lto PRIVATE $<TARGET_PROPERTY:os,COMPILE_DEFINITIONS>)

    target_compile_options(os_lto INTERFACE $<TARGET_PROPERTY:os,INTERFACE_COMPILE_OPTIONS>)
    target_compile_options(os_lto PRIVATE $<TARGET_PROPERTY:os,COMPILE_OPTIONS>)

    # Just an alias
    add_library(cmrx ALIAS os_lto)
else()
    add_library(os STATIC EXCLUDE_FROM_ALL ${os_SRCS})
    target_include_directories(os INTERFACE ${CMRX_ROOT_DIR}/include ${CMAKE_BINARY_DIR} ${HAL_PATH})

    # Just an alias
    add_library(cmrx ALIAS os)
endif()

if (UNIT_TESTING_BUILD)
    apply_gcov(os)
    add_subdirectory(tests)
endif()
