/*
 * CMRX RISC-V scheduler architecture support.
 *
 * Provides RISC-V implementations for scheduler primitives:
 * - Thread stack population
 * - Thread boot
 * - Kernel shutdown
 * - Syscall return value manipulation
 *
 */

#include <kernel/runtime.h>
#include <kernel/sched.h>
#include <kernel/arch/sched.h>
#include <arch/corelocal.h>
#include <cmrx/arch/riscv/exception_frame.h>
#include <cmrx/defines.h>
#include <stdint.h>
#include <string.h>

/*
 * Populate stack of new thread so it can be executed.
 *
 * Places an ExceptionFrame at the top of the thread stack.  When the
 * trap handler restores from this frame after a context switch, the
 * thread starts executing at 'entrypoint' with 'data' in a0 and
 * os_thread_dispose in ra.
 *
 * This follows the same pattern as ARM's os_thread_populate_stack
 * which builds an ExceptionFrame for PendSV to restore.
 */
uint32_t *os_thread_populate_stack(int stack_id, unsigned stack_size,
                                   entrypoint_t *entrypoint, void *data)
{
    uint32_t *stack = os_stack_get(stack_id);
    ExceptionFrame *frame = (ExceptionFrame *)&stack[stack_size - EXCEPTION_FRAME_WORDS];

    memset(frame, 0, sizeof(*frame));
    frame->ra      = (uint32_t)os_thread_dispose;
    frame->a0      = (uint32_t)data;
    frame->mepc    = (uint32_t)entrypoint;
    frame->mstatus = CMRX_RISCV_INITIAL_MSTATUS;

    return (uint32_t *)frame;
}

/** Platform-specific way of initializing threads.
 * Populates the stack of the thread with the initial values.
 * @param thread pointer to the thread to initialize
 * @param stack_size size of the stack
 * @param entrypoint address of the entrypoint
 * @param data pointer to the data
 */
void os_thread_initialize_arch(struct OS_thread_t * thread, unsigned stack_size, entrypoint_t * entrypoint, void * data)
{
    thread->sp = os_thread_populate_stack(thread->stack_id, stack_size, entrypoint, data);
}

/*
 * Create a process.
 *
 * For RISC-V without PMP/MPU implementation, this is a minimal stub
 * that just records the process definition.
 */
int os_process_create(Process_t process_id,
                      const struct OS_process_definition_t *definition)
{
    if (process_id >= OS_PROCESSES) {
        return E_OUT_OF_RANGE;
    }

    if (os_processes[process_id].definition != NULL) {
        return E_INVALID;
    }

    os_processes[process_id].definition = definition;

    /* No MPU configuration - PMP not implemented */

    return E_OK;
}

/*
 * Boot the first thread.
 *
 * Reads the ExceptionFrame from the thread stack and uses mret to
 * enter user code.  This mirrors the normal trap-return path: mepc
 * and mstatus are restored via CSR writes, then mret jumps to mepc
 * while promoting MPIE → MIE (enabling interrupts).
 */
__attribute__((noreturn))
void os_boot_thread(Thread_t boot_thread)
{
    struct OS_thread_t *thread = os_thread_get(boot_thread);
    ExceptionFrame *frame = (ExceptionFrame *)thread->sp;

    __asm__ volatile(
        "csrw mepc, %[mepc]\n\t"
        "csrw mstatus, %[mstatus]\n\t"
        "mv sp, %[sp_after]\n\t"
        "mv ra, %[ra]\n\t"
        "mv a0, %[a0]\n\t"
        "mret\n\t"
        :
        : [mepc] "r"(frame->mepc),
          [mstatus] "r"(frame->mstatus),
          [sp_after] "r"((uint32_t *)frame + EXCEPTION_FRAME_WORDS),
          [ra] "r"(frame->ra),
          [a0] "r"(frame->a0)
        : "memory"
    );

    __builtin_unreachable();
}

/*
 * Default CMRX shutdown handler.
 * This can be overridden by the application.
 */
__attribute__((weak, noreturn))
void cmrx_shutdown_handler(void)
{
    /* Default: infinite loop */
    while (1) {
        __asm__ volatile("wfi");
    }
}

/*
 * Perform the kernel shutdown.
 * Disables interrupts and calls the shutdown handler.
 */
__attribute__((noreturn))
void os_kernel_shutdown(void)
{
    os_core_lock();  /* Disable interrupts */
    cmrx_shutdown_handler();

    /* Never reached */
    __builtin_unreachable();
}

/*
 * Set the return value for a syscall on a specific thread.
 *
 * This is used for async syscalls where the return value is set
 * after the thread has been blocked and will resume later.
 *
 * For RISC-V, we need to modify a0 in the thread's saved context.
 */
int os_set_syscall_return_value(Thread_t thread_id, int32_t retval)
{
    struct OS_thread_t *thread = os_thread_get(thread_id);
    if (thread == NULL) {
        return E_INVALID;
    }

    ExceptionFrame *frame = (ExceptionFrame *)thread->sp;
    riscv_exception_set_retval(frame, retval);

    return E_OK;
}

/*
 * Put the CPU core to sleep.
 * Uses RISC-V WFI (Wait For Interrupt) instruction.
 */
void os_core_sleep(void)
{
    __asm__ volatile("wfi");
}
