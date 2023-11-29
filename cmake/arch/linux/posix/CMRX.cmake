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

function(target_link_libraries TGT_NAME)
    _target_link_libraries(${TGT_NAME} ${ARGN})
endfunction()

