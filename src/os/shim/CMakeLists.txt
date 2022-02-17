if (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${DEVICE_CPU})
    add_subdirectory(${DEVICE_CPU})
else()
	message(FATAL_ERROR "CPU ${DEVICE_CPU} is not supported by shim extensions!")
endif()

if (NOT TARGET os_shim_device)
	message(FATAL_ERROR "CPU-specific part of shim is misconfigured!")
endif()

set(shim_SRCS scb.c static.c clksource.c)
add_library(os_shim STATIC EXCLUDE_FROM_ALL ${shim_SRCS})
target_link_libraries(os_shim os_shim_device ${LIBOPENCM3})
