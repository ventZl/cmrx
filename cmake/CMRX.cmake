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
