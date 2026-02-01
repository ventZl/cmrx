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
#include <cmrx/defines.h>
#include <stdint.h>

/*
 * Populate stack of new thread so it can be executed.
 *
 * Sets up the stack frame so that when the thread is first scheduled,
 * it will start executing at the entrypoint with 'data' as argument.
 *
 * RISC-V calling convention (psABI):
 * - a0: first argument (data pointer)
 * - ra: return address (thread dispose handler)
 * - Stack must be 16-byte aligned
 *
 * Stack layout for first context switch (os_riscv_context_switch_perform):
 * The context switch saves/restores s0-s11 (12 words = 48 bytes).
 * For initial boot, we also need the exception-like frame that
 * os_boot_thread will use.
 *
 */
uint32_t *os_thread_populate_stack(int stack_id, unsigned stack_size,
                                   entrypoint_t *entrypoint, void *data)
{
    uint32_t *stack = os_stack_get(stack_id);

    /*
     * Initial stack layout (from top, stack_size words):
     *
     * stack[stack_size - 1]  : (padding for 16-byte alignment if needed)
     * stack[stack_size - 2]  : Initial PC (entrypoint)
     * stack[stack_size - 3]  : Initial ra (os_thread_dispose)
     * stack[stack_size - 4]  : Initial a0 (data)
     * stack[stack_size - 5]  : Reserved
     * stack[stack_size - 6]  : Reserved
     * stack[stack_size - 7]  : Reserved
     * stack[stack_size - 8]  : Reserved
     * stack[stack_size - 9]  : s11
     * stack[stack_size - 10] : s10
     * stack[stack_size - 11] : s9
     * stack[stack_size - 12] : s8
     * stack[stack_size - 13] : s7
     * stack[stack_size - 14] : s6
     * stack[stack_size - 15] : s5
     * stack[stack_size - 16] : s4
     * stack[stack_size - 17] : s3
     * stack[stack_size - 18] : s2
     * stack[stack_size - 19] : s1
     * stack[stack_size - 20] : s0
     *
     * SP will point to stack[stack_size - 20]
     */

    /* Clear the stack area we'll use */
    for (unsigned i = 0; i < 20; i++) {
        stack[stack_size - 1 - i] = 0;
    }

    /* Set up initial register values */
    stack[stack_size - 2] = (uint32_t)entrypoint;       /* Initial PC */
    stack[stack_size - 3] = (uint32_t)os_thread_dispose; /* ra - return to dispose */
    stack[stack_size - 4] = (uint32_t)data;              /* a0 - argument */

    /* s0-s11 are all zero (callee-saved, will be restored by context switch) */

    /* Return SP pointing to where context switch expects it */
    return &stack[stack_size - 20];
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
 * This function never returns. It sets up the CPU state and jumps
 * to the thread's entry point.
 *
 * For RISC-V, we:
 * 1. Load the thread's stack pointer
 * 2. Pop the initial register values
 * 3. Jump to the entry point
 */
__attribute__((noreturn))
void os_boot_thread(Thread_t boot_thread)
{
    struct OS_thread_t *thread = os_thread_get(boot_thread);
    uint32_t *thread_sp = thread->sp;

    /*
     * The stack was set up by os_thread_populate_stack.
     * Layout at thread_sp:
     *   +0:  s0
     *   +4:  s1
     *   ...
     *   +44: s11
     *   +48: reserved
     *   +52: reserved
     *   +56: reserved
     *   +60: a0 (argument)
     *   +64: ra (return address = os_thread_dispose)
     *   +68: PC (entry point)
     *
     * We need to:
     * 1. Skip past s0-s11 (they're just placeholders for first boot)
     * 2. Load a0, ra
     * 3. Jump to PC
     */

    __asm__ volatile(
        /* Set stack pointer to thread stack */
        "mv sp, %[sp]\n\t"

        /* Skip past callee-saved registers (s0-s11 = 12*4 = 48 bytes) */
        "addi sp, sp, 48\n\t"

        /* Skip reserved area (4 words = 16 bytes) */
        "addi sp, sp, 16\n\t"

        /* Load a0 (first argument) */
        "lw a0, 0(sp)\n\t"

        /* Load ra (return address) */
        "lw ra, 4(sp)\n\t"

        /* Load entry point into t0 */
        "lw t0, 8(sp)\n\t"

        /* Adjust sp past our boot frame */
        "addi sp, sp, 16\n\t"

        /* Jump to entry point */
        "jr t0\n\t"
        :
        : [sp] "r"(thread_sp)
        : "memory"
    );

    /* Never reached */
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

    /*
     * The thread's SP points to its saved context.
     * For RISC-V, the ecall handler saves the exception frame on stack.
     * The return value goes in a0.
     *
     * However, the exact stack layout depends on whether the thread
     * was blocked during ecall (exception frame) or IRQ (different frame).
     *
     * For now, this is a stub - proper implementation needs to understand
     * the exact stack layout for each case.
     */

    /* TODO: Proper implementation based on stack frame layout */
    (void)thread;
    (void)retval;

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
