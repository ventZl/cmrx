if (NOT TESTING)
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_code.ld "")
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_data.ld "")
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_bss.ld "")
	file(WRITE ${CMAKE_BINARY_DIR}/gen.appl_inst.ld "")

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
	  " LD_SCRIPT "${LD_SCRIPT}")
	  string(REPLACE "_data = .;" "_data = .;\n  INCLUDE gen.appl_data.ld;\n . = ALIGN(256);\n _e_data = .;" LD_SCRIPT "${LD_SCRIPT}")
	string(REPLACE ".bss : {" ". = ALIGN(512);\n _bss = .;\n .bss : {\n  INCLUDE gen.appl_bss.ld\n _e_bss = .;" LD_SCRIPT "${LD_SCRIPT}")
	if (SEMIHOSTING)
		string(REPLACE "/DISCARD/ : { *(.eh_frame) }" ".eh : { *(.eh_frame) }" LD_SCRIPT "${LD_SCRIPT}")
	endif()
	file(WRITE ${CMAKE_BINARY_DIR}/gen.${DEVICE}.ld "${LD_SCRIPT}")
endif()

macro(add_application NAME)
	if (NOT TESTING)
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_code.ld "/* Application name: ${NAME} */\n\t. = ALIGN(256);\n\t${NAME}_text_start = .;\n\tlib${NAME}.a(.text.*)\n\t. = ALIGN(256);\n\t${NAME}_text_end = .;\n\n")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_data.ld "/* Application name: ${NAME} */\n\t. = ALIGN(256);\n\t${NAME}_data_start = .;\n\tlib${NAME}.a(.data .data.*)\n\t. = ALIGN(256);\n\t${NAME}_data_end = .;\n\n")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_bss.ld "/* Application name: ${NAME} */\n\t. = ALIGN(256);\n\t${NAME}_bss_start = .;\n\tlib${NAME}.a(.bss .bss.*)\n\t. = ALIGN(256);\n\t${NAME}_bss_end = .;\n\n")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_inst.ld "/* Application name: ${NAME} */\n\tEXTERN(${NAME}_instance)\n\n")
	endif()
	add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${ARGN})
endmacro()
