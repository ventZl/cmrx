if (NOT TESTING)
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_code.ld "")
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_data.ld "")
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_bss.ld "")
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_inst.ld "")
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_vtable.ld "")
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_shared.ld "")

	file(READ ${CMAKE_BINARY_DIR}/gen.${DEVICE}.ld LD_SCRIPT)
	set(LD_SCRIPT "INCLUDE gen.appl_inst.ld\n${LD_SCRIPT}")
	string(REPLACE "*(.vectors)" "*(.vectors)\n  INCLUDE gen.appl_code.ld" LD_SCRIPT "${LD_SCRIPT}")
	string(REPLACE "*(.rodata*)" "*(.rodata*)
	  . = ALIGN(4);
	  __applications_start = .;
	  KEEP(*(.applications))
	  __applications_end = .;
	  __thread_create_start = .;
	  KEEP(*(.thread_create))
	  __thread_create_end = .;
	  INCLUDE gen.appl_vtable.ld;
	  " LD_SCRIPT "${LD_SCRIPT}")
	  string(REPLACE 
		  "_data = .;" 
		  "_data = .;
		  INCLUDE gen.appl_data.ld;
		  _e_data = .;
		  " LD_SCRIPT "${LD_SCRIPT}")
	string(REPLACE 
		".bss : {" 
		".shared : {
			INCLUDE gen.appl_shared.ld
		} > ram
		. = ALIGN(0x100);
		_bss = .;
		.bss : {
			INCLUDE gen.appl_bss.ld
			_e_bss = .;
		"
		LD_SCRIPT "${LD_SCRIPT}")
	if (SEMIHOSTING)
		string(REPLACE 
			"/DISCARD/ : { *(.eh_frame) }" 
			".eh : { *(.eh_frame) }" 
			LD_SCRIPT "${LD_SCRIPT}")
	endif()
	file(WRITE ${CMAKE_BINARY_DIR}/gen.${DEVICE}.ld "${LD_SCRIPT}")
else()
	include_directories(${CMAKE_SOURCE_DIR}/cmrx/include/testing)
endif()

function(add_application NAME)
	set(APPL "${NAME}#${CMAKE_CURRENT_BINARY_DIR}")
	get_property(CMRX_APPS GLOBAL PROPERTY CMRX_APP_LIST)
	list(APPEND CMRX_APPS ${APPL})
	set_property(GLOBAL PROPERTY CMRX_APP_LIST "${CMRX_APPS}")
	message("**** Applications: ${CMRX_APPS}")
	add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${ARGN})
	target_compile_definitions(${NAME} PRIVATE -D APPLICATION_NAME=${NAME})
endfunction()

add_event_hook(CMAKE_DONE ${CMAKE_CURRENT_LIST_DIR}/on_cmake_done.cmake)

macro(add_firmware FW_NAME)
	set_property(GLOBAL PROPERTY CMRX_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
	add_executable(${FW_NAME} ${ARGN})
	add_custom_command(TARGET ${FW_NAME} POST_BUILD
		COMMAND python ${CMAKE_SOURCE_DIR}/cmrx/ld/genlink.py ${CMAKE_CURRENT_BINARY_DIR}/helloworld.map ${CMAKE_BINARY_DIR}/gen.appl_data.ld ${CMAKE_BINARY_DIR}/gen.appl_bss.ld ${CMAKE_BINARY_DIR}/gen.appl_shared.ld

		COMMENT "Updating linker script for correct MPU operation"
		)
endmacro()
