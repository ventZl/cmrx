set(GDB_DEBUG_CONFIG "${CMAKE_CURRENT_LIST_DIR}/debug.gdb")
get_filename_component(GDB_CONFIG_DIR "${GDB_DEBUG_CONFIG}" DIRECTORY)

if (TESTING)
    # unit tests have to be built using native compiler,
    # so firmware building is disabled
    message(STATUS "Host-based testing enabled. Skipping HIL tests!")
    return()
endif()

if (NOT CMRX_GDB_PATH)
    message(STATUS "Path to GDB not set! Skipping tests! Set CMRX_GDB_PATH variable if you want to enable HIL tests.")
    return()
endif()

if (NOT EXISTS "${CMRX_GDB_PATH}")
    message(FATAL_ERROR "Debugger `${CMRX_GDB_PATH}` does not exist!")
endif()

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/openocd.cfg")
    message(FATAL_ERROR "File ${CMAKE_SOURCE_DIR}/openocd.cfg does not exist! Testing infrastructure needs it!")
endif()

if (NOT TARGET test_platform OR NOT TARGET test_platform_main)
    message(FATAL_ERROR "Host-based testing expects targets `test_platform` and `test_platform_main` being defined by the project which provides foundation test can link to to create working firmware binaries.")
endif()

message(STATUS "GDB: ${CMRX_GDB_PATH}")

find_tests(${CMAKE_CURRENT_SOURCE_DIR} ${CMRX_PLATFORM_TEST_SRC})
