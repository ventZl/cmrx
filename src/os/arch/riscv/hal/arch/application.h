#pragma once

/*
 * RISC-V HAL application metadata placement.
 *
 * The RP2350 RISC-V linker fragment defines .applications and .thread_create
 * sections and boundary symbols consumed by CMRX startup.
 */
#define CMRX_VTABLE_SPECIFIER __attribute__((section(".vtable."))) const

#define CMRX_APPLICATION_INSTANCE_ATTRIBUTES __attribute__((used, section(".applications")))
#define CMRX_THREAD_AUTOCREATE_ATTRIBUTES __attribute__((used, section(".thread_create")))

#define CMRX_APPLICATION_INSTANCE_CONSTRUCTOR(application) \
void * __APPL_SYMBOL(application, data_start) = (void *)1; \
void * __APPL_SYMBOL(application, data_end) = (void *)1; \
void * __APPL_SYMBOL(application, bss_start) = (void *)1; \
void * __APPL_SYMBOL(application, bss_end) = (void *)1; \
void * __APPL_SYMBOL(application, shared_start) = (void *)1; \
void * __APPL_SYMBOL(application, shared_end) = (void *)1; \
void * __APPL_SYMBOL(application, vtable_start) = (void *)1; \
void * __APPL_SYMBOL(application, vtable_end) = (void *)1; \
CMRX_APPLICATION_INSTANCE_ATTRIBUTES const struct OS_process_definition_t __APPL_SYMBOL(application, instance) = { \
    { \
        { &__APPL_SYMBOL(application, data_start), &__APPL_SYMBOL(application, data_end) }, \
        { &__APPL_SYMBOL(application, bss_start), &__APPL_SYMBOL(application, bss_end) }, \
        { __APPL_SYMBOL(application, mmio_start), __APPL_SYMBOL(application, mmio_end) }, \
        { __APPL_SYMBOL(application, mmio_2_start), __APPL_SYMBOL(application, mmio_2_end) }, \
        { &__APPL_SYMBOL(application, shared_start), &__APPL_SYMBOL(application, shared_end) } \
    }, \
    { &__APPL_SYMBOL(application, vtable_start), &__APPL_SYMBOL(application, vtable_end) } \
}

#define CMRX_THREAD_AUTOCREATE_CONSTRUCTOR(application, entrypoint, data, priority, core) \
CMRX_THREAD_AUTOCREATE_ATTRIBUTES const struct OS_thread_create_t __APPL_SYMBOL(application, thread_create_ ## entrypoint) = { \
    &__APPL_SYMBOL(application, instance), \
    entrypoint, \
    data, \
    priority, \
    core \
}
