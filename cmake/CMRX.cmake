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
string(REPLACE "_data = .;" "_data = .;\n  INCLUDE gen.appl_data.ld" LD_SCRIPT "${LD_SCRIPT}")
string(REPLACE ".bss : {" ".bss : {\n  INCLUDE gen.appl_bss.ld" LD_SCRIPT "${LD_SCRIPT}")
file(WRITE ${CMAKE_BINARY_DIR}/gen.${DEVICE}.ld "${LD_SCRIPT}")

function(add_application NAME)
	file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_code.ld "/* Application name: ${NAME} */\n\t. = ALIGN(256);\n\t${NAME}_text_start = .;\n\tlib${NAME}.a(.text.*)\n\t. = ALIGN(256);\n\t${NAME}_text_end = .;\n\n")
	file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_data.ld "/* Application name: ${NAME} */\n\t. = ALIGN(256);\n\t${NAME}_data_start = .;\n\tlib${NAME}.a(.data .data.*)\n\t. = ALIGN(256);\n\t${NAME}_data_end = .;\n\n")
	file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_bss.ld "/* Application name: ${NAME} */\n\t. = ALIGN(256);\n\t${NAME}_bss_start = .;\n\tlib${NAME}.a(.bss .bss.*)\n\t. = ALIGN(256);\n\t${NAME}_bss_end = .;\n\n")
	file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_inst.ld "/* Application name: ${NAME} */\n\tEXTERN(${NAME}_instance)\n\n")
	add_library(${NAME} STATIC ${ARGN})
endfunction()
