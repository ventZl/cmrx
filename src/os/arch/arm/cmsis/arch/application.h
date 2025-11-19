#pragma once

/* ARM Cortex-M specific way to define something a vtable */
#define CMRX_VTABLE_SPECIFIER __attribute__((section(".vtable."))) const

/* File intentionally left blank */
#define CMRX_APPLICATION_INSTANCE_ATTRIBUTES __attribute__((sed, section(".applications") ))
#define CMRX_THREAD_AUTOCREATE_ATTRIBUTES __attribute__((externally_visible, used, section(".thread_create") ))

#define CMRX_APPLICATION_INSTANCE_CONSTRUCTOR(application) \
extern void * __APPL_SYMBOL(application, data_start);\
extern void * __APPL_SYMBOL(application, data_end);\
extern void * __APPL_SYMBOL(application, bss_start);\
extern void * __APPL_SYMBOL(application, bss_end);\
extern void * __APPL_SYMBOL(application, vtable_start);\
extern void * __APPL_SYMBOL(application, vtable_end);\
extern void * __APPL_SYMBOL(application, __mmio_start);\
extern void * __APPL_SYMBOL(application, __mmio_end);\
extern void * __APPL_SYMBOL(application, shared_start);\
extern void * __APPL_SYMBOL(application, shared_end);\
\
CMRX_APPLICATION_INSTANCE_ATTRIBUTES const struct OS_process_definition_t __APPL_SYMBOL(application, instance) = {\
    {\
        { &__APPL_SYMBOL(application, data_start), &__APPL_SYMBOL(application, data_end) },\
        { &__APPL_SYMBOL(application, bss_start), &__APPL_SYMBOL(application, bss_end) },\
        { __APPL_SYMBOL(application, mmio_start), __APPL_SYMBOL(application, mmio_end) },\
        { __APPL_SYMBOL(application, mmio_2_start), __APPL_SYMBOL(application, mmio_2_end) },\
        { &__APPL_SYMBOL(application, shared_start), &__APPL_SYMBOL(application, shared_end) }\
    },\
    { &__APPL_SYMBOL(application, vtable_start), &__APPL_SYMBOL(application, vtable_end) }\
};

#define CMRX_THREAD_AUTOCREATE_CONSTRUCTOR(application, entrypoint, data, priority, core) \
CMRX_THREAD_AUTOCREATE_ATTRIBUTES const struct OS_thread_create_t __APPL_SYMBOL(application, thread_create_ ## entrypoint) = {\
    &__APPL_SYMBOL(application, instance),\
    entrypoint,\
    data,\
    priority,\
    core\
};
