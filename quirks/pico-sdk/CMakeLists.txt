get_target_property(SRCS pico_standard_link INTERFACE_SOURCES)
list(FILTER SRCS EXCLUDE REGEX crt0)
list(APPEND SRCS "${CMAKE_CURRENT_LIST_DIR}/crt0.S")
message(STATUS "pico_sdlib sources: ${SRCS}")
set_target_properties(pico_standard_link PROPERTIES INTERFACE_SOURCES "${SRCS}")

target_sources(os PRIVATE ${CMAKE_CURRENT_LIST_DIR}/cmrx_isrs.c)
