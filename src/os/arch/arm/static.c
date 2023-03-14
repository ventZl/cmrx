#include "cmrx/os/runtime.h"
#include <cmrx/arch/arm/common/static.h>

extern const struct OS_process_definition_t __applications_start;
extern const struct OS_process_definition_t __applications_end;

extern const struct OS_thread_create_t __thread_create_start;
extern const struct OS_thread_create_t __thread_create_end;

unsigned static_init_thread_count()
{
    return &__thread_create_end - &__thread_create_start;
}

const struct OS_thread_create_t * const static_init_thread_table()
{
    return &__thread_create_start;

}

unsigned static_init_process_count()
{
    return &__applications_end - &__applications_start;

}

const struct OS_process_definition_t * const static_init_process_table()
{
    return &__applications_start;
}

