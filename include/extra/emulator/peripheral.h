#pragma once

#include <stdint.h>

/** Simulate register read.
 *
 * Use this macro to access emulated peripheral register. It will trigger
 * emulator callback before the data is read. The callback is executed
 * *before* the data is accessed, so the callback has the opportunity to
 * modify the value of register and application will see this modification.
 *
 * @param instance address of the peripheral instance
 * @param reg name of peripheral register read. May be 8-, 16- or 32-bits large
 * @returns value of that register as provided by the peripheral
 */
#define _R(instance, reg)   _Generic((instance->reg), \
uint8_t : cmrx_emu_reg_R8,\
uint16_t : cmrx_emu_reg_R16,\
uint32_t : cmrx_emu_reg_R32)(instance, &instance->reg)

/** Simulate register write.
 *
 * Use this macro to access emulated peripheral register. It will trigger
 * emulator callback after the data is written. The callback is executed
 * *after* the data is accessed, so the callback will see the modified
 * content of the register.
 *
 * @param instance address of the peripheral instance
 * @param reg name of peripheral register read. May be 8-, 16- or 32-bits large
 * @param value value to be written. Type must match the register size.
 */
#define _W(instance, reg, value)  _Generic((instance->reg), \
uint8_t : cmrx_emu_reg_W8,\
uint16_t : cmrx_emu_reg_W16,\
uint32_t : cmrx_emu_reg_W32)(instance, &instance->reg, value)

uint8_t cmrx_emu_reg_R8(void * instance, const volatile uint8_t * source);
uint16_t cmrx_emu_reg_R16(void * instance, const volatile uint16_t * source);
uint32_t cmrx_emu_reg_R32(void * instance, const volatile uint32_t * source);

void cmrx_emu_reg_W8(void * instance, volatile uint8_t * dest, uint8_t value);
void cmrx_emu_reg_W16(void * instance, volatile uint16_t * dest, uint16_t value);
void cmrx_emu_reg_W32(void * instance, volatile uint32_t * dest, uint32_t value);
