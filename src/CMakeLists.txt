if (NOT UNIT_TESTING_BUILD)
    add_subdirectory(extra)
endif()
add_subdirectory(os)
if (NOT UNIT_TESTING_BUILD)
    add_subdirectory(lib)
endif()
if (UNIT_TESTING_BUILD)
    add_subdirectory(testing)
endif()
