function(add_firmware FW_NAME)
    add_executable(${FW_NAME} EXCLUDE_FROM_ALL ${ARGN})
    # Do nothing
endfunction()

function(target_add_applications TGT_NAME)
    # Do nothing
endfunction()
