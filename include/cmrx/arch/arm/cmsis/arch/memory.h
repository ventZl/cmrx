#pragma once

/* This symbol is placed exactly at the address where FLASH starts */
extern void * __cmrx_flash_origin;

/* This symbol is placed at the address of FLASH length */
extern void * __cmrx_flash_length;

#define code_base() (&__cmrx_flash_origin)
#define code_size() ((int)(&__cmrx_flash_length))


