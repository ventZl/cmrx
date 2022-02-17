#include <cmrx/shim/static.h>

#include <cmrx/os/runtime.h>

/* Pointers to process definition array in firmware image
 * Linker script defines two symbols __applications_start and
 * __applications_end. These mark boundaries for processes
 * defined at compile time (by use of OS_APPLICATION macros).
 */
extern const struct OS_process_definition_t __applications_start;
extern const struct OS_process_definition_t __applications_end;

/* Pointers to thread autocreate definition array in firmware
 * image. Linker script defines two symbols __thread_create_start and
 * __thread_create_end. These mark boundaries of thread autocreate
 * list defined at compile time (by use of OS_THREAD_CREATE marco).
 */
extern const struct OS_thread_create_t __thread_create_start;
extern const struct OS_thread_create_t __thread_create_end;

unsigned static_init_thread_count()
{
	return &__thread_create_end - &__thread_create_start;
}

unsigned static_init_process_count()
{
	return &__applications_end - &__applications_start;
}

const struct OS_thread_create_t * const static_init_thread_table()
{
	return &__thread_create_start;
}

const struct OS_process_definition_t * const static_init_process_table()
{
	return &__applications_start;
}


