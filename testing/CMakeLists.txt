add_library(emu STATIC EXCLUDE_FROM_ALL emu.c)
get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
target_include_directories(emu PUBLIC ${CMRX_ROOT_DIR}/include ${CMRX_ROOT_DIR}/src/os/arch/testing ${CMAKE_BINARY_DIR})
