#pragma once

/** @ingroup arch_linux_impl
 * @{
 */

/** Internal routine to register application with kernel.
 * This routine is used by the static creation mechanism to
 * register application with kernel. It will be called before
 * the main() is executed. It puts application definition into
 * list of known application definitions for the kernel to
 * create this process upon kernel startup.
 * @param process process definition structure
 * @note This function is kernel-private and specific to Linux port
 */
extern void cmrx_posix_register_application(const struct OS_process_definition_t * process);

/** Internal routine to register thread with kernel.
 * This routine is used by the thread autostart mechanism to
 * register thread with kernel. It will be called before
 * the main() is executed. It puts thread definition into
 * list of known thread definitions for the kernel to
 * create this thread upon kernel startup.
 * @param process thread definition structure
 * @note This function is kernel-private and specific to Linux port
 */
extern void cmrx_posix_register_thread(const struct OS_thread_create_t * thread);

/** @}
 */

/** @defgroup arch_linux Linux port
 * @ingroup arch
 * Linux port provides the ability to run CMRX-based environment hosted on
 * ordinary Linux machine as a ordinary userspace process.
 *
 * Linux architecture support is an environment where CMRX scheduler
 * and kernel API runs hosted as ordinary Linux process. Here CMRX uses
 * Linux' own syscalls to implement functionality of the abstract machine.
 *
 * The environment which Linux-based environment creates encapsulates
 * whole system into one process. All CMRX processes share common address
 * space. This is similar to other architectures which CMRX currently
 * supports.
 * @{
 */

/** Linux port implementation of application creation macro.
 *
 * As of now this macro does nothing. It is here just to make
 * this port conformal to the API.
 */
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

/** Linux port implementation of thread autostart.
 *
 * Thread autostart is implemented by creating instance of structure
 * describing thread to be created and creating a function that calls
 * @ref cmrx_posix_register_thread. This function is marked as constructor
 * which ensures it is called before the main is started.
 *
 * See @ref OS_THREAD_CREATE for more details in arguments.
 */
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

/**
 * @}
 */
