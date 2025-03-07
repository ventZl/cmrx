message(STATUS "CMSIS CMRX CMake component loaded")

function(__cmrx_get_linker_script_for_device DEVICE OUTPUT)
    set(${OUTPUT} "${CMAKE_BINARY_DIR}/gen.${DEVICE}.ld" PARENT_SCOPE)
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

    get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
    if ("${CMRX_ROOT_DIR}" STREQUAL "")
        message(FATAL_ERROR "CMRX source tree directory property not set! Did you forget to add_subdirectory(cmrx) ?")
    endif()

	if (TESTING)
		set(EXCL EXCLUDE_FROM_ALL)
	endif()
	add_executable(${FW_NAME} ${EXCL} ${ARGN})
    add_custom_command(TARGET ${FW_NAME} POST_BUILD
        COMMAND ${PYTHON_EXE} ${CMRX_ROOT_DIR}/ld/genlink-cmsis.py --realign
            ${CMAKE_CURRENT_BINARY_DIR}/$<TARGET_FILE_NAME:${FW_NAME}>.map 
            ${FW_NAME}
            ${CMAKE_CURRENT_BINARY_DIR}

    	COMMENT "Updating linker script for correct MPU operation"
    	)
    set_property(TARGET ${FW_NAME} PROPERTY CMRX_IS_FIRMWARE 1)

    # DONE: Call genline-cmsis.py here to modify gen.${DEVICE}.ld to create gen.${FW_NAME}.${DEVICE}.ld containing
    # callouts to different included sub-linker scripts.
    # file(COPY_FILE ${CMAKE_BINARY_DIR}/gen.${DEVICE}.ld ${CMAKE_BINARY_DIR}/gen.${FW_NAME}.${DEVICE}.ld)
    execute_process(
        COMMAND ${PYTHON_EXE} ${CMRX_ROOT_DIR}/ld/genlink-cmsis.py --create
        ${DEVICE_LINKER_SCRIPT}
        ${BINARY_LINKER_SCRIPT}
        ${FW_NAME}
        )
    target_link_options(${FW_NAME} PUBLIC -Wl,-Map=${FW_NAME}.map)
    file(CREATE_LINK ${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.map ${CMAKE_CURRENT_BINARY_DIR}/${FW_NAME}.elf.map SYMBOLIC)
    # TODO: CMake will de-duplicate any incoming interface
    if (NOT CMRX_SKIP_LINKER_FILE_USE)
        target_link_options(${FW_NAME} PUBLIC LINKER:--script=${BINARY_LINKER_SCRIPT})
    endif()
    set_property(TARGET ${FW_NAME} PROPERTY PICO_TARGET_LINKER_SCRIPT ${BINARY_LINKER_SCRIPT})
endfunction()

function(target_add_applications TGT_NAME)
    target_link_libraries(${TGT_NAME} ${ARGN})

    get_target_property(IS_FIRMWARE ${TGT_NAME} CMRX_IS_FIRMWARE)
    if ("${IS_FIRMWARE}" EQUAL "1")
        get_property(CMRX_ROOT_DIR GLOBAL PROPERTY CMRX_ROOT_DIR)
        #        message(STATUS "${TGT_NAME} is a firmware, iterating over libraries")
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
                #       message(STATUS "${LIBRARY} is an application, adding into linker script")
                execute_process(
                    COMMAND ${PYTHON_EXE} ${CMRX_ROOT_DIR}/ld/genlink-cmsis.py --add-application
                        ${OUT_DIR}lib${LIBRARY}.a
                        ${TGT_NAME}
                        ${CMAKE_CURRENT_BINARY_DIR} 
                    )
            endif()
        endforeach()
    endif()
endfunction()


