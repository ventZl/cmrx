#pragma once

#include <conf/kernel.h>

#ifdef CMRX_CUSTOM_FLASH_RANGE
/** This symbol is placed exactly at the address where FLASH starts by the linker script */
extern void * __cmrx_flash_origin;

/** This symbol is placed at the address of FLASH length by the linker script */
extern void * __cmrx_flash_length;

/** Obtain base address of all the .text in the flash image */
#define code_base() (&__cmrx_flash_origin)

/** Obtain size of all the .text in the flash image */
#define code_size() ((int)(&__cmrx_flash_length))

#else

#define code_base() (CMRX_CUSTOM_FLASH_START)
#define code_size() (CMRX_CUSTOM_FLASH_SIZE)

#endif

