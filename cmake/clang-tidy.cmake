find_program(CLANGTIDY_EXECUTABLE clang-tidy)

if (CMRX_CLANG_TIDY AND "${CLANGTIDY_EXECUTABLE}" STREQUAL "CLANGTIDY_EXECUTABLE-NOTFOUND")
    message(STATUS "Clang-tidy not found! Linting disabled!")
endif()

function(apply_clang_tidy TARGET)
    if (NOT CMRX_CLANG_TIDY OR "${CLANGTIDY_EXECUTABLE}" STREQUAL "CLANGTIDY_EXECUTABLE-NOTFOUND")
        return()
    endif()

    if (NOT TARGET ${TARGET})
        message(FATAL_ERROR "apply_clang_tidy: ${TARGET} is not a valid target name!")
    endif()

    if (NOT TARGET clang-tidy)
        add_custom_target(clang-tidy)
    endif()

    get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)

    if ("${CMRX_ROOT_DIR}" STREQUAL "")
        message(FATAL_ERROR "apply_clang_tidy: CMRX root dir property not set!")
    endif()

    get_target_property(TARGET_SOURCE_DIR ${TARGET} SOURCE_DIR)

    get_target_property(TARGET_SOURCES ${TARGET} SOURCES)

    add_custom_target(tidy-${TARGET}
        COMMAND ${CLANGTIDY_EXECUTABLE}
        --extra-arg=-I
        --extra-arg=/usr/include
        -config-file=${CMRX_ROOT_DIR}/.clang-tidy
        -p ${CMAKE_BINARY_DIR}
        ${TARGET_SOURCES}
        WORKING_DIRECTORY ${TARGET_SOURCE_DIR}
        COMMENT "Linting ${TARGET}..."
    )

    add_dependencies(clang-tidy tidy-${TARGET})
endfunction()
