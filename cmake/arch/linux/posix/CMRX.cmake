message(STATUS "POSIX CMRX CMake component loaded")

## Add firmware binary definition 
# This function is a wrapper around add_executable, which will augment the firmware binary 
# with both necessary and useful attachments:
# * map file generation will be commanded 
# * custom linker script will for target binary
# * post-build commands to update linker script for correct MPU operation
function(add_firmware FW_NAME)
	if (TESTING)
		set(EXCL EXCLUDE_FROM_ALL)
	endif()
	add_executable(${FW_NAME} ${EXCL} ${ARGN})
    set_property(TARGET ${FW_NAME} PROPERTY CMRX_IS_FIRMWARE 1)
	target_link_options(${FW_NAME} PUBLIC -Wl,-Map=${FW_NAME}.map)
endfunction()

function(target_add_applications TGT_NAME)
    target_link_libraries(${TGT_NAME} ${ARGN})
endfunction()

## Add firmware application definition
# This function is a wrapper around add_library(), which will augment the library to act
# as a CMRX application.
function(add_application NAME)
	add_library(${NAME} OBJECT EXCLUDE_FROM_ALL ${ARGN})
	set_property(TARGET ${NAME} PROPERTY CMRX_IS_APPLICATION 1)
    get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
	target_compile_definitions(${NAME} PRIVATE -D APPLICATION_NAME=${NAME})
endfunction()
