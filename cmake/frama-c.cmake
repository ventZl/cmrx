find_program(FRAMAC_EXECUTABLE frama-c)

if (CMRX_FRAMAC AND "${FRAMAC_EXECUTABLE}" STREQUAL "FRAMAC_EXECUTABLE-NOTFOUND")
	message(STATUS "Frama-C not found! Formal verification disabled!")
else()
	message(STATUS "Frama-C found: ${FRAMAC_EXECUTABLE}")
endif()

function(apply_framac TARGET)
    if (NOT CMRX_CLANG_TIDY OR "${FRAMAC_EXECUTABLE}" STREQUAL "FRAMAC_EXECUTABLE-NOTFOUND")
        return()
    endif()

    if (NOT TARGET ${TARGET})
        message(FATAL_ERROR "apply_framac: ${TARGET} is not a valid target name!")
    endif()

    if (NOT TARGET framac)
        add_custom_target(frama-c)
    endif()

    get_target_property(TARGET_SOURCE_DIR ${TARGET} SOURCE_DIR)

    get_target_property(TARGET_SOURCES ${TARGET} SOURCES)

    add_custom_target(frama-c-${TARGET}
	    COMMAND ${FRAMAC_EXECUTABLE} -main os_system_call -wp -wp-rte -compilation-db ${CMAKE_BINARY_DIR} $<TARGET_PROPERTY:${TARGET},SOURCES> -then -mdr-gen md -mdr-out ${CMAKE_BINARY_DIR}/verification.md -save ${CMAKE_BINARY_DIR}/cmrx.framac
        WORKING_DIRECTORY ${TARGET_SOURCE_DIR}
	COMMAND_EXPAND_LISTS
	COMMENT "Formal verification of ${TARGET}..."
    )

    add_dependencies(frama-c frama-c-${TARGET})
endfunction()
