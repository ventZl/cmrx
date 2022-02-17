add_subdirectory(kernel)

if (NOT TESTING)
	add_subdirectory(shim)
endif()
