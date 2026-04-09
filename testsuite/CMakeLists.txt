if (CMRX_INTEGRATION_TESTS)
    # Integration tests use HIL tester infrastructure
    set(CMRX_HIL_TESTS ON)
endif()


if (NOT CMRX_HIL_TESTS)
    return()
endif()


set(GDB_DEBUG_CONFIG "${CMAKE_CURRENT_LIST_DIR}/debug.gdb")
get_filename_component(GDB_CONFIG_DIR "${GDB_DEBUG_CONFIG}" DIRECTORY)

if (NOT CMRX_GDB_PATH)
    find_program(CMRX_GDB_PATH NAMES gdb DOC "GDB executable")
    if ("${CMRX_GDB_PATH}" STREQUAL "CMRX_GDB_PATH-NOTFOUND")
        message(FATAL_ERROR "GDB executable not found! Set CMRX_GDB_PATH variable if you want to enable HIL tests.")
    endif()
endif()

if (NOT EXISTS "${CMRX_GDB_PATH}")
    message(FATAL_ERROR "Debugger `${CMRX_GDB_PATH}` does not exist!")
endif()

if (NOT CMRX_HW_TESTING_SKIP_OPENOCD)
    if (NOT EXISTS "${CMAKE_SOURCE_DIR}/openocd.cfg")
        message(FATAL_ERROR "File ${CMAKE_SOURCE_DIR}/openocd.cfg does not exist! Testing infrastructure needs it!")
    endif()
endif()

if (NOT TARGET test_platform OR NOT TARGET test_platform_main)
    message(FATAL_ERROR "Host-based testing expects targets `test_platform` and `test_platform_main` being defined by the project which provides foundation test can link to to create working firmware binaries.")
endif()

message(STATUS "GDB: ${CMRX_GDB_PATH}")

# Create test harness target that can be used by others to create their own tests
add_library(hil_test_harness STATIC debug.c)
target_link_directories(hil_test_harness PUBLIC ${CMAKE_CURRENT_LIST_DIR})

if (CMRX_INTEGRATION_TESTS)
    find_tests(${CMAKE_CURRENT_SOURCE_DIR} ${CMRX_PLATFORM_TEST_SRC})
endif()
