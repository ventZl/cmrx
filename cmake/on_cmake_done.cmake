function(_unpack PACKED FIRST SECOND)
	string(FIND "${PACKED}" "#" HASH_POS)
	string(SUBSTRING "${PACKED}" 0 ${HASH_POS} FIRST_VAL)
	math(EXPR AFTER_HASH_POS "${HASH_POS} + 1")
	string(SUBSTRING "${PACKED}" ${AFTER_HASH_POS} -1 SECOND_VAL)
	set(${FIRST} "${FIRST_VAL}" PARENT_SCOPE)
	set(${SECOND} "${SECOND_VAL}" PARENT_SCOPE)
endfunction()

get_property(ELF_DIR GLOBAL PROPERTY CMRX_BINARY_DIR)
get_property(CMRX_APPS GLOBAL PROPERTY CMRX_APP_LIST)
message("#### Applications: ${CMRX_APPS}")
foreach(APP ${CMRX_APPS})
	_unpack("${APP}" LIBNAME LIBDIR)
	message("%%%%\nFile: ${LIBNAME}\nDirectory: ${LIBDIR}")
	file(RELATIVE_PATH ODIR ${ELF_DIR} ${LIBDIR})
	if (NOT "${ODIR}" STREQUAL "")
		set(ODIR "${ODIR}/")
	endif()

	if (NOT TESTING)
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_code.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_text_start = .;
			${ODIR}lib${LIBNAME}.a(.text.*)
			. = ALIGN(4);
			${LIBNAME}_text_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_data.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_data_start = .;
			${ODIR}lib${LIBNAME}.a(.data .data.*)
			. = ALIGN(4);
			${LIBNAME}_data_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_bss.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_bss_start = .;
			${ODIR}lib${LIBNAME}.a(.bss .bss.*)
			. = ALIGN(4);
			${LIBNAME}_bss_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_inst.ld 
			"/* Application name: ${LIBNAME} */
			EXTERN(${LIBNAME}_instance)

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_vtable.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_vtable_start = .;
			${ODIR}lib${LIBNAME}.a(.vtable .vtable.*)
			. = ALIGN(4);
			${LIBNAME}_vtable_end = .;

			")
		file(APPEND ${CMAKE_BINARY_DIR}/gen.appl_shared.ld 
			"/* Application name: ${LIBNAME} */
			. = ALIGN(4);
			${LIBNAME}_shared_start = .;
			${ODIR}lib${LIBNAME}.a(.shared .shared.*)
			. = ALIGN(4);
			${LIBNAME}_shared_end = .;

			")
	endif()
endforeach()


