#pragma once

#include <stdint.h>

/** Generate emulated interrupt.
 *
 * Trigger interrupt from emulated peripheral. This can be used
 * to deliver interrupts to the software running inside CMRX.
 *
 * Avoid calling handlers directly as that could cause handlers
 * to be executing concurrently to the CMRX thread - a situation
 * currently not supported!
 *
 * @param irq_no Number of the interrupt. Valid values are 0 - 15
 *
 * @note Interrupts have fixed priorities with interrupt with numeric
 * lower value having higher priority. Currently, preemption is not
 * implemented and all interrupts are serialized.
 *
 * You can handle the interrupt on the application side by creating
 * a function:
 *
 * void IRQ_`no`() {}
 *
 * If it is defined, then calling `generate_interrupt` will call
 * it. It will be called by interrupting the currently running
 * CMRX thread and preempting the IRQ handler. The same way real
 * hardware does it.
 */
void generate_interrupt(unsigned irq_no);

typedef struct {
    void (*emu_write_cb)(volatile void * addr);
    void (*emu_read_cb)(const volatile void * addr);
} CMRX_Peripheral_Emulator_t ;

/** Create peripheral emulator.
 *
 * Peripheral emulator is an object that encapsulates peripheral instance
 * and provides callback for reading and writing operation.
 *
 * You can then use @ref _W and @ref _R functions to emulate register
 * and writing.
 *
 * @param peripheral_type C type name that defines register structure of
 * the peripheral. This may look like CMSIS peripheral definition. All
 * members of this structure should be defined volatile, they may optionally
 * be defined as const, if application software is not supposed to write
 * into these specific registers.
 * @param name name of the emulator object
 * @param read_cb name of the function that provides register read callback
 * @param write_cb name of the function that provides register write callback
 *
 * @note You have to declare one additional type named Full_`peripheral_type`
 * which has the same layout and size as the `peripheral_type` which describes
 * your peripheral. This type may drop all `const` specifiers in member
 * definitions.
 *
 * The purpose of this type is that emulator will overlay both normal and
 * "full" type. This way you can write from within the peripheral emulator
 * even into registers which are actually not writable by the application.
 */
#define CMRX_PERIPHERAL_EMULATOR(peripheral_type, name, read_cb, write_cb) \
_Static_assert(sizeof(Full_ ## peripheral_type) == sizeof(peripheral_type), "Type describing peripheral and type that gives full access to it must both have the same size!");\
typedef struct __attribute__((packed)) {\
    CMRX_Peripheral_Emulator_t emu;\
    union {\
        peripheral_type device;\
        Full_ ## peripheral_type device_full;\
    };\
} Emu_ ## peripheral_type;\
\
Emu_ ## peripheral_type name = {\
    .emu = { .emu_write_cb = write_cb, .emu_read_cb = read_cb }\
}
