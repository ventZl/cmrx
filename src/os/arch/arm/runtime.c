#include <kernel/runtime.h>
#include <arch/runtime.h>

#ifdef __FPU_USED
void os_thread_initialize_arch(struct OS_thread_t * thread)
{
    // By default, thread is restored into
    // Thread mode, using PSP as a stack and
    // without FPU
    thread->arch.fp_active = 0;
}
#endif

bool os_get_fpu_active_in_thread(Thread_t thread_id)
{
    struct OS_thread_t * thread = &os_threads[thread_id];
    uint8_t fpu_active = thread->arch.fp_active;
    uint8_t rpc_depth = rpc_stack_depth(thread_id);
    return (fpu_active > rpc_depth) & 1;
}
