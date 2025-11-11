#pragma once

extern void cmrx_posix_register_application(const struct OS_process_definition_t * process);
extern void cmrx_posix_register_thread(const struct OS_thread_create_t * thread);

#define CMRX_APPLICATION_INSTANCE_CONSTRUCTOR(application) \
void * __APPL_SYMBOL(application, data_start) = (void *) 1;\
void * __APPL_SYMBOL(application, data_end) = (void *) 1;\
void * __APPL_SYMBOL(application, bss_start) = (void *) 1;\
void * __APPL_SYMBOL(application, bss_end) = (void *) 1;\
void * __APPL_SYMBOL(application, shared_start) = (void *) 1;\
void * __APPL_SYMBOL(application, shared_end) = (void *) 1;\
void * __APPL_SYMBOL(application, vtable_start) = (void *) 1;\
void * __APPL_SYMBOL(application, vtable_end) = (void *) 1;\
\
const struct OS_process_definition_t __APPL_SYMBOL(application, instance) = {\
    {\
        { &__APPL_SYMBOL(application, data_start), &__APPL_SYMBOL(application, data_end) },\
        { &__APPL_SYMBOL(application, bss_start), &__APPL_SYMBOL(application, bss_end) },\
        { __APPL_SYMBOL(application, mmio_start), __APPL_SYMBOL(application, mmio_end) },\
        { __APPL_SYMBOL(application, mmio_2_start), __APPL_SYMBOL(application, mmio_2_end) },\
        { &__APPL_SYMBOL(application, shared_start), &__APPL_SYMBOL(application, shared_end) }\
    },\
    { &__APPL_SYMBOL(application, vtable_start), &__APPL_SYMBOL(application, vtable_end) }\
};\
__attribute__((constructor)) void __APPL_SYMBOL(application, inst_construct)(void)\
{\
    cmrx_posix_register_application(&__APPL_SYMBOL(application, instance));\
}

#define CMRX_THREAD_AUTOCREATE_CONSTRUCTOR(application, entrypoint, data, priority, core) \
const struct OS_thread_create_t __APPL_SYMBOL(application, thread_create_ ## entrypoint) = {\
    &__APPL_SYMBOL(application, instance),\
    entrypoint,\
    data,\
    priority,\
    core\
}; \
__attribute__((constructor)) void __APPL_SYMBOL(application, thread_create_ ## entrypoint ## _construct)(void)\
{\
    cmrx_posix_register_thread(&__APPL_SYMBOL(application, thread_create_ ## entrypoint));\
}
