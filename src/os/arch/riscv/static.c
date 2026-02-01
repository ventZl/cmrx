/*
 * CMRX RISC-V static initialization.
 *
 * Provides functions to retrieve static initialization structures
 * for threads and processes defined at compile time.
 *
 * These functions rely on linker-generated symbols that mark the
 * boundaries of the application and thread tables.
 */

#include <kernel/runtime.h>
#include <kernel/arch/static.h>

/*
 * Linker-generated symbols marking the boundaries of the
 * statically-defined application and thread tables.
 *
 * These are populated by the OS_APPLICATION and OS_THREAD_CREATE macros.
 */
extern const struct OS_process_definition_t __applications_start;
extern const struct OS_process_definition_t __applications_end;

extern const struct OS_thread_create_t __thread_create_start;
extern const struct OS_thread_create_t __thread_create_end;

/*
 * Get the count of statically-defined threads.
 */
unsigned static_init_thread_count(void)
{
    return &__thread_create_end - &__thread_create_start;
}

/*
 * Get pointer to the static thread creation table.
 */
const struct OS_thread_create_t *static_init_thread_table(void)
{
    return &__thread_create_start;
}

/*
 * Get the count of statically-defined processes/applications.
 */
unsigned static_init_process_count(void)
{
    return &__applications_end - &__applications_start;
}

/*
 * Get pointer to the static process definition table.
 */
const struct OS_process_definition_t *static_init_process_table(void)
{
    return &__applications_start;
}
