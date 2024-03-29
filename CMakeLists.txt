message(STATUS "CMRX root dir: ${CMAKE_CURRENT_SOURCE_DIR}")
set_property(GLOBAL PROPERTY CMRX_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(include)
include_directories(.)
include_directories(${CMAKE_BINARY_DIR})
add_definitions(-Wall -Wextra)
set(CMAKE_C_STANDARD 11)

if (NOT CMRX_ARCH)
    message(FATAL_ERROR "Target architecture not defined!")
endif()

if (NOT CMRX_HAL)
    message(FATAL_ERROR "HAL not selected!")
endif()

set(HAL_PATH ${CMAKE_CURRENT_SOURCE_DIR}/include/cmrx/arch/${CMRX_ARCH}/${CMRX_HAL})
get_filename_component(HAL_PATH ${HAL_PATH} ABSOLUTE)
message(STATUS "HAL path: ${HAL_PATH}")
if (NOT IS_DIRECTORY ${HAL_PATH})
    message(FATAL_ERROR "Architecture ${CMRX_ARCH} does not support HAL ${CMRX_HAL}!")
endif()

include_directories(${HAL_PATH})

if (TESTING)
    include_directories(ctest)
	include_directories(include/testing)
endif()

configure_file(conf/kernel.h ${CMAKE_BINARY_DIR}/conf/kernel.h)
add_subdirectory(src)
add_subdirectory(testsuite)

