find_program(GCOV_EXECUTABLE gcov)

if ("${GCOV_EXECUTABLE}" STREQUAL "GCOV_EXECUTABLE-NOTFOUND")
    message(STATUS "GCOV not found! Linting disabled!")
    message(STATUS "GCOV found: ${GCOV_EXECUTABLE}")
endif()

# Adds GCOV coverage to the target via compiler switches
function(apply_gcov TARGET)
    if ("${GCOV_EXECUTABLE}" STREQUAL "GCOV_EXECUTABLE-NOTFOUND")
        return()
    endif()

    if (NOT TARGET ${TARGET})
        message(FATAL_ERROR "apply_gcov: ${TARGET} is not a valid target name!")
    endif()

    target_compile_options(${TARGET} PRIVATE -ftest-coverage -fprofile-arcs -g)
    target_link_options(${TARGET} INTERFACE -lgcov --coverage)
endfunction()

# Performs coverage report generation
function(generate_coverage)
    if ("${GCOV_EXECUTABLE}" STREQUAL "GCOV_EXECUTABLE-NOTFOUND")
        return()
    endif()

    if (NOT TARGET coverage)
        add_custom_target(coverage
            COMMAND lcov --capture --directory ${CMAKE_BINARY_DIR} --output-file ${CMAKE_BINARY_DIR}/coverage.info
            COMMAND genhtml ${CMAKE_BINARY_DIR}/coverage.info -output ${CMAKE_BINARY_DIR}/coverage
            )
    endif()
endfunction()
