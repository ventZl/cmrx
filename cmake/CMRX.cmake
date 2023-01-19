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

## Add firmware application definition
# This function is a wrapper around add_library(), which will augment the library to act 
# as a CMRX application.
function(add_application NAME)
	add_library(${NAME} STATIC EXCLUDE_FROM_ALL ${ARGN})
	set_property(TARGET ${NAME} PROPERTY CMRX_IS_APPLICATION 1)
	target_compile_definitions(${NAME} PRIVATE -D APPLICATION_NAME=${NAME})
endfunction()

function(__cmrx_get_linker_script_for_device DEVICE OUTPUT)
    set(${OUTPUT} "${CMAKE_CURRENT_BINARY_DIR}/gen.${DEVICE}.ld" PARENT_SCOPE)
endfunction()

function(__cmrx_get_linker_script_for_binary DEVICE FW_NAME OUTPUT)
    set(${OUTPUT} "${CMAKE_CURRENT_BINARY_DIR}/gen.${DEVICE}.${FW_NAME}.ld" PARENT_SCOPE)
endfunction()

## Add firmware binary definition 
# This function is a wrapper around add_executable, which will augment the firmware binary 
# with both necessary and useful attachments:
# * map file generation will be commanded 
# * custom linker script will for target binary
# * post-build commands to update linker script for correct MPU operation
function(add_firmware FW_NAME)
    __cmrx_get_linker_script_for_device(${DEVICE} DEVICE_LINKER_SCRIPT)
    __cmrx_get_linker_script_for_binary(${DEVICE} ${FW_NAME} BINARY_LINKER_SCRIPT)
	get_property(CMRX_BINARIES GLOBAL PROPERTY CMRX_BINARY_LIST)
	list(APPEND CMRX_BINARIES ${FW_NAME})
	set_property(GLOBAL PROPERTY CMRX_BINARY_LIST "${CMRX_BINARIES}")

	set_property(GLOBAL PROPERTY CMRX_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR})
	if (TESTING)
		set(EXCL EXCLUDE_FROM_ALL)
	endif()
	add_executable(${FW_NAME} ${EXCL} ${ARGN})
	add_custom_command(TARGET ${FW_NAME} POST_BUILD
        COMMAND python ${CMAKE_SOURCE_DIR}/cmrx/ld/genlink-cmsis.py --realign
            ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:${FW_NAME}>.map 
            ${FW_NAME}
            ${CMAKE_BINARY_DIR}

		COMMENT "Updating linker script for correct MPU operation"
		)
    set_property(TARGET ${FW_NAME} PROPERTY CMRX_IS_FIRMWARE 1)

    # DONE: Call genline-cmsis.py here to modify gen.${DEVICE}.ld to create gen.${FW_NAME}.${DEVICE}.ld containing
    # callouts to different included sub-linker scripts.
    # file(COPY_FILE ${CMAKE_BINARY_DIR}/gen.${DEVICE}.ld ${CMAKE_BINARY_DIR}/gen.${FW_NAME}.${DEVICE}.ld)
    execute_process(
        COMMAND python ${CMAKE_SOURCE_DIR}/cmrx/ld/genlink-cmsis.py --create 
        ${DEVICE_LINKER_SCRIPT}
        ${BINARY_LINKER_SCRIPT}
        ${FW_NAME}
        )
	target_link_options(${FW_NAME} PUBLIC -Wl,-Map=${FW_NAME}.map)
    # TODO: CMake will de-duplicate any incoming interface 
    target_link_options(${FW_NAME} PUBLIC LINKER:--script=${BINARY_LINKER_SCRIPT})
    set_property(TARGET ${FW_NAME} PROPERTY PICO_TARGET_LINKER_SCRIPT ${BINARY_LINKER_SCRIPT})
endfunction()

function(target_link_libraries TGT_NAME)
    _target_link_libraries(${TGT_NAME} ${ARGN})

    get_target_property(IS_FIRMWARE ${TGT_NAME} CMRX_IS_FIRMWARE)
    if ("${IS_FIRMWARE}" EQUAL "1")
        message(STATUS "${TGT_NAME} is a firmware, iterating over libraries")
        __cmrx_get_linker_script_for_binary(${DEVICE} ${TGT_NAME} BINARY_LINKER_SCRIPT)
        foreach(LIBRARY ${ARGN})
            get_target_property(IS_APPLICATION ${LIBRARY} CMRX_IS_APPLICATION)
            get_target_property(LIB_DIR ${LIBRARY} BINARY_DIR)
            get_target_property(ELF_DIR ${TGT_NAME} BINARY_DIR)
            file(RELATIVE_PATH OUT_DIR ${ELF_DIR} ${LIB_DIR})
            if (NOT "${OUT_DIR}" STREQUAL "")
                set (OUT_DIR "${OUT_DIR}/")
            endif()
            if ("${IS_APPLICATION}" EQUAL "1")
                message(STATUS "${LIBRARY} is an application, adding into linker script")
                execute_process(
                    COMMAND python ${CMAKE_SOURCE_DIR}/cmrx/ld/genlink-cmsis.py --add-application 
                        ${OUT_DIR}lib${LIBRARY}.a
                        ${TGT_NAME}
                        ${CMAKE_BINARY_DIR} 
                    )
            endif()
        endforeach()
    endif()
endfunction()
