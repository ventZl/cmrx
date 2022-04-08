function(_unpack PACKED FIRST SECOND)
	string(FIND "${PACKED}" "#" HASH_POS)
	string(SUBSTRING "${PACKED}" 0 ${HASH_POS} FIRST_VAL)
	math(EXPR AFTER_HASH_POS "${HASH_POS} + 1")
	string(SUBSTRING "${PACKED}" ${AFTER_HASH_POS} -1 SECOND_VAL)
	set(${FIRST} "${FIRST_VAL}" PARENT_SCOPE)
	set(${SECOND} "${SECOND_VAL}" PARENT_SCOPE)
endfunction()

function(_patch_linker_script DEVICE BINARY)
	if (NOT TESTING)
		file(WRITE ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_code.ld "")
		file(WRITE ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_data.ld "")
		file(WRITE ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_bss.ld "")
		file(WRITE ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_inst.ld "")
		file(WRITE ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_vtable.ld "")
		file(WRITE ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_shared.ld "")

		file(READ ${CMAKE_BINARY_DIR}/gen.${DEVICE}.ld LD_SCRIPT)
		set(LD_SCRIPT "INCLUDE gen.${BINARY}.appl_inst.ld\n${LD_SCRIPT}")
		string(REPLACE "*(.vectors)" "*(.vectors)\n  INCLUDE gen.${BINARY}.appl_code.ld" LD_SCRIPT "${LD_SCRIPT}")
		string(REPLACE "*(.rodata*)" "*(.rodata*)
		  . = ALIGN(4);
		  __applications_start = .;
		  KEEP(*(.applications))
		  __applications_end = .;
		  __thread_create_start = .;
		  KEEP(*(.thread_create))
		  __thread_create_end = .;
		  INCLUDE gen.${BINARY}.appl_vtable.ld;
		  " LD_SCRIPT "${LD_SCRIPT}")
		  string(REPLACE 
			  "_data = .;" 
			  "_data = .;
			  INCLUDE gen.${BINARY}.appl_data.ld;
			  _e_data = .;
			  " LD_SCRIPT "${LD_SCRIPT}")
		string(REPLACE 
			".bss : {" 
			".shared : {
				INCLUDE gen.${BINARY}.appl_shared.ld
			} > ram
			. = ALIGN(0x100);
			_bss = .;
			.bss : {
				INCLUDE gen.${BINARY}.appl_bss.ld
				_e_bss = .;
			"
			LD_SCRIPT "${LD_SCRIPT}")
		if (SEMIHOSTING)
			string(REPLACE 
				"/DISCARD/ : { *(.eh_frame) }" 
				".eh : { *(.eh_frame) }" 
				LD_SCRIPT "${LD_SCRIPT}")
		endif()
		file(WRITE ${CMAKE_BINARY_DIR}/gen.${BINARY}.${DEVICE}.ld "${LD_SCRIPT}")
		#	else()
		#		include_directories(${CMAKE_SOURCE_DIR}/cmrx/include/testing)
	endif()
endfunction()

function(_gen_app_linker_script LIBNAME BINARY)
	if (NOT TARGET ${LIBNAME} OR NOT TARGET ${BINARY})
		message(FATAL_ERROR "Both application and binary must be valid existing CMake targets!")
	endif()
	get_property(ELF_DIR TARGET ${BINARY} PROPERTY BINARY_DIR)
	get_property(LIBDIR TARGET ${LIBNAME} PROPERTY BINARY_DIR)
	message("ELF is built into ${ELF_DIR}")
	message("APP is built into ${LIBDIR}")
	file(RELATIVE_PATH ODIR ${ELF_DIR} ${LIBDIR})
	if (NOT "${ODIR}" STREQUAL "")
		set(ODIR "${ODIR}/")
	endif()

	if (NOT TESTING)
		file(APPEND ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_code.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_text_start = .;
			${ODIR}lib${LIBNAME}.a(.text.*)
			. = ALIGN(4);
			${LIBNAME}_text_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_data.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_data_start = .;
			${ODIR}lib${LIBNAME}.a(.data .data.*)
			. = ALIGN(4);
			${LIBNAME}_data_end = .;

			")
			file(APPEND ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_bss.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_bss_start = .;
			${ODIR}lib${LIBNAME}.a(.bss .bss.*)
			. = ALIGN(4);
			${LIBNAME}_bss_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_inst.ld 
			"/* Application name: ${LIBNAME} */
			EXTERN(${LIBNAME}_instance)

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_vtable.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_vtable_start = .;
			${ODIR}lib${LIBNAME}.a(.vtable .vtable.*)
			. = ALIGN(4);
			${LIBNAME}_vtable_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.${BINARY}.appl_shared.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_shared_start = .;
			${ODIR}lib${LIBNAME}.a(.shared .shared.*)
			. = ALIGN(4);
			${LIBNAME}_shared_end = .;

			")
	endif()
endfunction()

function(add_application NAME)
	set(APPL "${NAME}#${CMAKE_CURRENT_BINARY_DIR}")
	get_property(CMRX_APPS GLOBAL PROPERTY CMRX_APP_LIST)
	list(APPEND CMRX_APPS ${APPL})
	set_property(GLOBAL PROPERTY CMRX_APP_LIST "${CMRX_APPS}")
	add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${ARGN})
	set_property(TARGET ${NAME} PROPERTY CMRX_IS_APPLICATION 1)
	target_compile_definitions(${NAME} PRIVATE -D APPLICATION_NAME=${NAME})
endfunction()

add_event_hook(CMAKE_DONE ${CMAKE_CURRENT_LIST_DIR}/on_cmake_done.cmake)

function(add_firmware FW_NAME)
	get_property(CMRX_BINARIES GLOBAL PROPERTY CMRX_BINARY_LIST)
	list(APPEND CMRX_BINARIES ${FW_NAME})
	message("${CMRX_BINARIES}")
	set_property(GLOBAL PROPERTY CMRX_BINARY_LIST "${CMRX_BINARIES}")

	set_property(GLOBAL PROPERTY CMRX_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
	if (TESTING)
		set(EXCL EXCLUDE_FROM_ALL)
	endif()
	add_executable(${FW_NAME} ${EXCL} ${ARGN})
	add_custom_command(TARGET ${FW_NAME} POST_BUILD
		COMMAND python ${CMAKE_SOURCE_DIR}/cmrx/ld/genlink.py ${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.map ${CMAKE_BINARY_DIR}/gen.${FW_NAME}.appl_data.ld ${CMAKE_BINARY_DIR}/gen.${FW_NAME}.appl_bss.ld ${CMAKE_BINARY_DIR}/gen.${FW_NAME}.appl_shared.ld

		COMMENT "Updating linker script for correct MPU operation"
		)
	target_link_options(${FW_NAME} PUBLIC -Wl,-Map=${FW_NAME}.map)
	target_link_options(${FW_NAME} PUBLIC -T${CMAKE_BINARY_DIR}/gen.${FW_NAME}.${DEVICE}.ld)
endfunction()

