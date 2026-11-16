# Copyright (C) 2021 Eduard Drusa
# SPDX-License-Identifier: MPL-2.0

if (NOT EXISTS CACHE{GCC_EXE})
	find_program(GCC_EXE arm-none-eabi-gcc
		PATHS
		/opt/arm/bin
		/opt/arm-none-eabi/bin
		ENV CROSS_GCC_PATH
		DOC "ARM cross-compiler toolchain location")
endif()

if (NOT EXISTS CACHE{GXX_EXE})
	find_program(GXX_EXE arm-none-eabi-g++
		PATHS
		/opt/arm/bin
		/opt/arm-none-eabi/bin
		ENV CROSS_GXX_PATH
		DOC "ARM cross-compiler toolchain location")
endif()

if (NOT EXISTS CACHE{LD_EXE})
	find_program(LD_EXE arm-none-eabi-ld
		PATHS
		/opt/arm/bin
		/opt/arm-none-eabi/bin
		ENV CROSS_GCC_PATH
		DOC "ARM cross-compiler linker location")
endif()

if (NOT EXISTS CACHE{OBJCOPY_EXE})
	find_program(OBJCOPY_EXE arm-none-eabi-objcopy
		PATHS
		/opt/arm/bin
		/opt/arm-none-eabi/bin
		ENV CROSS_GCC_PATH
		DOC "ARM objcopy location")
endif()

if (NOT EXISTS CACHE{AR_EXE})
	find_program(AR_EXE arm-none-eabi-gcc-ar
		PATHS
		/opt/arm/bin
		/opt/arm-none-eabi/bin
		ENV CROSS_GCC_PATH
		DOC "ARM archiver location")
endif()

if (NOT EXISTS CACHE{RANLIB_EXE})
	find_program(RANLIB_EXE arm-none-eabi-gcc-ranlib
		PATHS
		/opt/arm/bin
		/opt/arm-none-eabi/bin
		ENV CROSS_GCC_PATH
		DOC "ARM ranlib location")
endif()

if ("${GCC_EXE}" STREQUAL "GCC_EXE-NOTFOUND")
	message(FATAL_ERROR "Unable to find arm-none-eabi GCC toolchain! Either place it in one of following locations:\n/opt/arm/bin\n/usr/bin\n/usr/local/bin\nor provide its path in CROSS_GCC_PATH environment variable!")
endif()

set(CMAKE_C_COMPILER "${GCC_EXE}")
set(CMAKE_CXX_COMPILER "${GXX_EXE}")
set(CMAKE_ASM_COMPILER "${GCC_EXE}")
set(CMAKE_C_LINKER "${LD_EXE}")
set(CMAKE_AR "${AR_EXE}")
set(CMAKE_RANLIB "${RANLIB_EXE}")
set(CMAKE_SYSTEM_NAME "Generic")
set(CMAKE_SYSTEM_PROCESSOR "arm")
set(CMAKE_CROSSCOMPILING TRUE)
set(CMAKE_EXECUTABLE_SUFFIX_C ".elf")

if (NOT _NRF_FAMILY OR NOT _NRF_PART)
	message(FATAL_ERROR "Missing Nordic Semiconductor part identifiers!")
endif()

if (NOT "${_NRF_FAMILY}" STREQUAL "nrf52")
	set(TARGET_FLAGS "-mcpu=cortex-m33 -mfpu=fpv4-sp-d16 -mfloat-abi=hard ")
else()
	string(SUBSTRING "${_NRF_PART}" 2 8 _NRF_PART_NO)
	if ("${_NRF_PART_NO}" LESS "52832")
		set(TARGET_FLAGS "-mcpu=cortex-m4 -mfloat-abi=soft ")
	else()
		set(TARGET_FLAGS "-mcpu=cortex-m4 -mfpu=fpv4-sp-d16 -mfloat-abi=hard ")
	endif()
endif()

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${TARGET_FLAGS}")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic -fdata-sections -ffunction-sections")
if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0 -g3")
endif()
if(CMAKE_BUILD_TYPE MATCHES Release)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Os -g0")
endif()

set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} --specs=nosys.specs")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--gc-sections")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--start-group -lc -lm -Wl,--end-group")
set(CMAKE_C_LINK_FLAGS "${CMAKE_C_LINK_FLAGS} -Wl,--print-memory-usage")
