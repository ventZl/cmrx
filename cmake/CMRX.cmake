if ("${CMRX_ARCH}" STREQUAL "")
    message(FATAL_ERROR "CMRX_ARCH not defined! Please define target architecture to be used!")
endif()

if ("${CMRX_HAL}" STREQUAL "")
    message(FATAL_ERROR "CMRX_HAL not defined! Please define target HAL to be used!")
endif()

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/arch/${CMRX_ARCH}/${CMRX_HAL}/CMRX.cmake)
    include(${CMAKE_CURRENT_LIST_DIR}/arch/${CMRX_ARCH}/${CMRX_HAL}/CMRX.cmake)
    #else()
    #message("File ${CMAKE_CURRENT_LIST_DIR}/arch/${CMRX_ARCH}/${CMRX_HAL}/CMRX.cmake not found, skipping!")
endif()

## Add firmware application definition
# This function is a wrapper around add_library(), which will augment the library to act 
# as a CMRX application.
function(add_application NAME)
	add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${ARGN})
	set_property(TARGET ${NAME} PROPERTY CMRX_IS_APPLICATION 1)
	target_compile_definitions(${NAME} PRIVATE -D APPLICATION_NAME=${NAME})
endfunction()


