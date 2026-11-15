set(CMRX_CUSTOM_FLASH_RANGE TRUE)
set(CMRX_CUSTOM_FLASH_START 0x0)
set(CMRX_CUSTOM_FLASH_SIZE 0x20000000)

# RP2xxx platform-specific callback for add_firmware
# By default, Pico-SDK uses divider peripheral to perform division.
# That makes random access to divider peripheral from userspace context
# which trips MPU. This callback will override this setting and lets
# compiler emit its own division code. This is slower but works with
# MPU.
function(add_firmware_platform_cb FW_NAME)
    message(STATUS "Disabling divider peripheral for `${FW_NAME}`")
    pico_set_divider_implementation(${FW_NAME} compiler)
endfunction()
