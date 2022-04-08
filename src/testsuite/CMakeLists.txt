find_file(GDB_DEBUG_CONFIG NAMES debug.gdb
	PATHS ${CMAKE_CURRENT_LIST_DIR}
	REQUIRED
	)

get_filename_component(GDB_CONFIG_DIR "${GDB_DEBUG_CONFIG}" DIRECTORY)

function(add_hw_test binary)
	if (TARGET ${binary})
		message(STATUS "Add test: ${binary}")
		add_test(NAME ${binary}
			COMMAND timeout 60 /opt/arm-none-eabi/bin/arm-none-eabi-gdb -batch -x ${GDB_DEBUG_CONFIG} $<TARGET_FILE:${binary}>
			WORKING_DIRECTORY ${GDB_CONFIG_DIR}
			)
	else()
		message(FATAL_ERROR "Unknown target ${binary}! Unable to create hardware-based test for it!")
	endif()
endfunction()

add_subdirectory(os_start)
