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

macro(add_application NAME)
	if (NOT TESTING)
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_code.ld 
			"/* Application name: ${NAME} */
			. = ALIGN(4);
			${NAME}_text_start = .;
			lib${NAME}.a(.text.*)
			. = ALIGN(4);
			${NAME}_text_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_data.ld 
			"/* Application name: ${NAME} */
			. = ALIGN(4);
			${NAME}_data_start = .;
			lib${NAME}.a(.data .data.*)
			. = ALIGN(4);
			${NAME}_data_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_bss.ld 
			"/* Application name: ${NAME} */
			. = ALIGN(4);
			${NAME}_bss_start = .;
			lib${NAME}.a(.bss .bss.*)
			. = ALIGN(4);
			${NAME}_bss_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_inst.ld 
			"/* Application name: ${NAME} */
			EXTERN(${NAME}_instance)

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_vtable.ld 
			"/* Application name: ${NAME} */
			. = ALIGN(4);
			${NAME}_vtable_start = .;
			lib${NAME}.a(.vtable .vtable.*)
			. = ALIGN(4);
			${NAME}_vtable_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_shared.ld 
			"/* Application name: ${NAME} */
			. = ALIGN(4);
			${NAME}_shared_start = .;
			lib${NAME}.a(.shared .shared.*)
			. = ALIGN(4);
			${NAME}_shared_end = .;

			")
	endif()
	add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${ARGN})
	target_compile_definitions(${NAME} PRIVATE -D APPLICATION_NAME=${NAME})
endmacro()

macro(add_firmware FW_NAME)
	add_executable(${FW_NAME} ${ARGN})
	add_custom_command(TARGET ${FW_NAME} POST_BUILD
		COMMAND python ${CMAKE_SOURCE_DIR}/cmrx/ld/genlink.py ${CMAKE_CURRENT_BINARY_DIR}/helloworld.map ${CMAKE_BINARY_DIR}/gen.appl_data.ld ${CMAKE_BINARY_DIR}/gen.appl_bss.ld ${CMAKE_BINARY_DIR}/gen.appl_shared.ld

		COMMENT "Updating linker script for correct MPU operation"
		)
endmacro()
