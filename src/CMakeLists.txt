if (TESTING)
	include_directories(../include/testing)
	add_subdirectory(testing)
endif()

add_subdirectory(os)
add_subdirectory(lib)
