set(GDB_DEBUG_CONFIG "${CMAKE_CURRENT_LIST_DIR}/debug.gdb")
get_filename_component(GDB_CONFIG_DIR "${GDB_DEBUG_CONFIG}" DIRECTORY)

if (TESTING)
    # unit tests have to be built using native compiler,
    # so firmware building is disabled
    message(STATUS "Host-based testing enabled. Skipping HIL tests!")
    return()
endif()

if (NOT CMRX_GDB_PATH)
    message(STATUS "Path to GDB not set! Skipping tests!")
    return()
endif()

if (NOT EXISTS "${CMRX_GDB_PATH}")
    message(FATAL_ERROR "Debugger `${CMRX_GDB_PATH}` does not exist!")
endif()

if (NOT EXISTS "${CMAKE_SOURCE_DIR}/openocd.cfg")
    message(FATAL_ERROR "File ${CMAKE_SOURCE_DIR}/openocd.cfg does not exist! Testing infrastructure needs it!")
endif()

message(STATUS "GDB: ${CMRX_GDB_PATH}")

find_tests(${CMAKE_CURRENT_SOURCE_DIR})
