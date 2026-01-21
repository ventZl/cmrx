#define _GNU_SOURCE
#include <kernel/arch/mpu.h>
#include <kernel/arch/sched.h>
#include <kernel/arch/context.h>
#include <kernel/rpc.h>
#include <kernel/syscall.h>
#include <cmrx/sys/syscalls.h>
#include <kernel/sched.h>
#include <conf/kernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <threads.h>
#include <pthread.h>
#include <assert.h>
#include "linux.h"
#include "clock.h"
#include "thread.h"
#include "interrupt.h"
#include "relay.h"

/** @defgroup arch_linux_impl Implementation details
 * @ingroup arch_linux
 *
 * This group contains platform-specific routines that implement
 * internal behavior of this port.
 *
 * Each CMRX thread is backed by one Linux thread. CMRX kernel is using
 * signals and blocking syscalls to enforce preemption on threads.
 *
 * Internally the architecture of the port is similar to the ARM Cortex-M
 * one. Kernel is using timers "interrupt" and "thread switch interrupt".
 * These "interrupts" are serviced by kernel thread synchronously, so they
 * never preempt each other. This makes sure that threads are never
 * switched while inside syscall.
 * @{
 */


/** Shall thread switch be performed?
 * While kernel timer is called periodically, it doesn't perform
 * thread switch inside the handler. Rather it schedules thread
 * switch interrupt which is then executed synchronously by the
 * "kernel" thread.
 * This flag holds information if this thread switch should be
 * requested or not. Due to the semantics of CMRX it is possible
 * that this flag will be activated and then deactivated before
 * thread switch happens (e.g. because interrupt caused thread
 * wakeup before thread managed to be scheduled out of CPU).
 */
static volatile bool kernel_execute_thread_switch = false;

/** List of registered syscalls.
 *
 * This list contains syscall definitions that were registered with
 * kernel. The registration happens automatically if @ref REGISTER_SYSCALLS
 * macro was used.
 */
static struct Syscall_Entry_t syscalls[256] = { 0 };
/** Counter for known syscalls.
 * This counter holds amount of syscalls known to the kernel.
 * @note This does not relate to syscall number, where highest syscall number
 * may be higher than this count. It should never be lower though.
 */
static unsigned syscalls_count = 0;

/** Trigger thread switch if requested.
 *
 * Triggers thread switch interrupt if requested and clears the flag.
 * @param thread_stopped true if thread is already stopped (e.g. due to entering system call)
 * @note This should always be ran in CMRX thread context
 */
void trigger_pendsv_if_needed()
{
    assert(current_thread_id != -1);
    assert(os_threads[current_thread_id].arch.sched_thread_id == pthread_self());

    if (kernel_execute_thread_switch)
    {
        request_pending_service();
        kernel_execute_thread_switch = false;
    }
}

/** Entrypoint into system call machinery.
 *
 * This function will fill thread-private buffer for system call data
 * and will dispatch the system call handler to be called.
 */
int system_call_entrypoint(unsigned long arg0,
                            unsigned long arg1,
                            unsigned long arg2,
                            unsigned long arg3,
                            unsigned long arg4,
                            unsigned long arg5,
                            unsigned char syscall_id)
{
    (void) arg4;
    (void) arg5;

    struct syscall_dispatch_t * syscall = &os_threads[current_thread_id].arch.syscall;
    memset(syscall, 0, sizeof(struct syscall_dispatch_t));
    syscall->args[0] = arg0;
    syscall->args[1] = arg1;
    syscall->args[2] = arg2;
    syscall->args[3] = arg3;
    syscall->args[4] = arg4;
    syscall->args[5] = arg5;
    syscall->retval = ~0;
    syscall->syscall_id = syscall_id;
    syscall->retval = E_NOTAVAIL;

    enter_system_call();

    int rv = syscall->retval;

    if (syscall_id == SYSCALL_RPC_CALL)
    {
        assert(syscall->outcome == SYSCALL_OUTCOME_RPC_CALL);
    }
    switch (syscall->outcome) {
        case SYSCALL_OUTCOME_RETURN:
            return rv;
            break;

        case SYSCALL_OUTCOME_RPC_CALL:
            rv = syscall->dispatch_target(
                (RPC_Service_t *) syscall->dispatch_args[0],
                syscall->dispatch_args[1],
                syscall->dispatch_args[2],
                syscall->dispatch_args[3],
                syscall->dispatch_args[4]
            );
            system_call_entrypoint(rv, 0, 0, 0, 0, 0, SYSCALL_RPC_RETURN);
            break;

        default:
            assert(0);
    }
    return rv;
}

/** This function handles the heavy lifting of thread switching.
 *
 * Another thread is woken-up by unblocking it, then this thread is suspended.
 */
void thread_switch_handler(int signo)
{
    assert(signo == SIGUSR1);
    is_cmrx_thread();

    /* Check that we are running in the context of thread that
     * CMRX considers the one that is currently running.
     */
    volatile struct OS_core_state_t * cpu_state = &core[coreid()];
    assert(cpu_state->thread_current == current_thread_id);

    // Spurious execution of this handler
    // This may happen as signals don't behave the same as interrupts
    if (cpu_state->thread_current == cpu_state->thread_next)
    {
        return;
    }

    if (os_threads[cpu_state->thread_current].state == THREAD_STATE_RUNNING)
    {
        // only mark leaving thread as ready, if it was runnig before
        // if leaving thread was, for example, quit before calling
        // os_sched_yield, then this would return it back to life
        os_threads[cpu_state->thread_current].state = THREAD_STATE_READY;
    }

    cpu_state->thread_prev = cpu_state->thread_current;
    cpu_state->thread_current = cpu_state->thread_next;

    os_threads[cpu_state->thread_current].state = THREAD_STATE_RUNNING;

    thread_resume_execution(cpu_state->thread_current);

    // This thread is stopped now
    thread_suspend_execution(false);

    // Here we were resumed by someone else.
    // Wait until that someone else is suspended
    while (!os_threads[cpu_state->thread_prev].arch.is_suspended) {}

    // Check if there is a signal pending
    if (os_threads[cpu_state->thread_current].signals != 0 && os_threads[cpu_state->thread_current].signal_handler != NULL)
    {
        os_threads[cpu_state->thread_current].signal_handler(os_threads[cpu_state->thread_current].signals);
        os_threads[cpu_state->thread_current].signals = 0;
    }
}

/** Handler for kernel service call.
 *
 * This handler handles kernel syscall execution. The fact that kernel
 * syscalls are called via this handler makes sure that:
 * 1. kernel syscall is never interrupted via timer
 * 2. only one thread is ever inside syscall
 */
void kernel_service_handler(int signo)
{
    assert(signo == SIGURG);
    assert(current_thread_id != -1);

    /* Check that we are running in the context of thread that
     * CMRX considers the one that is currently running.
     */
    volatile struct OS_core_state_t * cpu_state = &core[coreid()];
    assert(cpu_state->thread_current == current_thread_id);

    struct syscall_dispatch_t * syscall = &os_threads[current_thread_id].arch.syscall;

    syscall->outcome = SYSCALL_OUTCOME_RETURN;
    syscall->retval = os_system_call(syscall->args[0], syscall->args[1], syscall->args[2], syscall->args[3], syscall->syscall_id);

    trigger_pendsv_if_needed();
}

/** Routine to jump-start guest thread host.
 *
 * This routine will configure the Linux thread to act as a host
 * for CMRX thread. We don't want '
 * This is a Linux thread that hosts guest thread.
 * We don't want these threads to process timer signals.
 * These are basically ever only sensitive to SIGUSR1 which
 * is used to force preemption.
 */
int thread_startup_handler(void * arg)
{
    struct thread_startup_t * startup_data = (struct thread_startup_t *) arg;
    assert(startup_data != NULL);
    assert(startup_data->thread_id >= 0 && startup_data->thread_id <= OS_THREADS);

    current_thread_id = startup_data->thread_id;
    assert(current_thread_id != -1);

    os_threads[current_thread_id].arch.sched_thread_id = pthread_self();

#ifndef __APPLE__
    char thread_name[17];
    snprintf(thread_name, 16, "CMRX thread: %d", startup_data->thread_id);
    pthread_setname_np(os_threads[current_thread_id].arch.sched_thread_id, thread_name);
#endif
    /** Allow reception of SIGALRM, SIGUSR1 (PendSV) and SIGURG (SVCHandler)
     * signals in CMRX thread hosting threads */
    sigset_t set;

    // Perform initial thread block, so we can create all threads up and front
    // without having any of them being actually executed
    thread_suspend_execution(true);

    // Check that this thread wasn't resumed spuriously
    volatile struct OS_core_state_t * cpu_state = &core[coreid()];
    assert(cpu_state->thread_current == current_thread_id);

    /* Now that the thread was unblocked by initial resume,
     * we will enable signal arrival. Signals are configured
     * to block each other out and once threads are inside
     * signal handlers, they won't accept signal preempting another
     * signal even if these are unblocked.
     */
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGUSR2);
    sigaddset(&set, SIGURG);

    pthread_sigmask(SIG_UNBLOCK, &set, NULL);

    int thread_rv = startup_data->entry_point(startup_data->entry_arg);
    free(startup_data);

    linux_thread_exit(thread_rv);

    return thread_rv;
}

/** Register syscall with kernel.
 *
 * The call to this function is arranged by each block where
 * syscalls are defined. It registers all syscalls provided in
 * syscall registration request.
 *
 * This function is called during constructor phase, before
 * main was executed.
 */
void cmrx_posix_register_syscalls(struct Syscall_Entry_t * added_syscalls)
{
    while (!(added_syscalls->handler == 0 && added_syscalls->id == 0))
    {
        syscalls[syscalls_count++] = *added_syscalls;
        added_syscalls++;
    }
}

/**
 * @}
 */

/** @ingroup arch_linux
 * @{
 */

/** Configure Linux architecture behavior.
 * We will use SIGUSR1 handler as a machinery to actually switch threads.
 * This routine will find the next thread, pause current one and then restart the other.
 *
 * The SIGUSR2 handler serves the purpose of forcing the thread to cooperate
 * on thread switching. Whenever a thread is commanded SIGUSR2, it will enter waiting
 * and will remain waiting until there is a command to resume execution.
 *
 * This is the next best thing that can be done in order to simulate thread scheduling.
 */
void os_init_arch(void)
{
    /* PendSV should "disable interrupts", which means
     * that it blocks SIGURG, SIGUSR1 and SIGALRM */
    struct sigaction sigusr1_action = { 0 };
    sigemptyset(&sigusr1_action.sa_mask);
    sigaddset(&sigusr1_action.sa_mask, SIGURG);
    sigaddset(&sigusr1_action.sa_mask, SIGUSR2);
    sigaddset(&sigusr1_action.sa_mask, SIGALRM);
    sigusr1_action.sa_flags = 0;
    sigusr1_action.sa_handler = &thread_switch_handler;

    /* SIGURG has higher priority than SIGUSR1 (PendSV)
     */
    struct sigaction sigurg_action = { 0 };
    sigemptyset(&sigurg_action.sa_mask);
    sigaddset(&sigurg_action.sa_mask, SIGUSR1);
    // TODO: Timer should still be able to preempt the kernel
    sigaddset(&sigurg_action.sa_mask, SIGALRM);
    sigurg_action.sa_flags = 0;
    sigurg_action.sa_handler = &kernel_service_handler;

    if (sigaction(SIGUSR1, &sigusr1_action, NULL) != 0)
    {
        perror("Unable to install SIGUSR1 handler: ");
        abort();
    }

    if (sigaction(SIGURG, &sigurg_action, NULL) != 0)
    {
        perror("Unable to install SIGURG handler: ");
        abort();
    }

    sigset_t set;

    /* Disable delivery of these signals in all threads that are spawned from
     * this thread onwards.
     *
     * We will selectively enable them in threads that host CMRX threads to
     * recreate semantics similar to
     */
    sigemptyset(&set);
    sigaddset(&set, SIGALRM);
    sigaddset(&set, SIGUSR1);
    sigaddset(&set, SIGURG);
    pthread_sigmask(SIG_BLOCK, &set, NULL);
    printf("SIGALRM, SIGUSR1 and SIGURG masked\n");

    irq_init();
}

/** Perform platform-specific initialization of thread.
 *
 * This will initialize facilities for thread synchronization.
 * Here we will create pipe pair used to block the thread
 * on external request and the Linux thread is created for the
 * CMRX thread.
 */
void os_thread_initialize_arch(struct OS_thread_t * thread, unsigned stack_size, entrypoint_t * entrypoint, void * data)
{
    (void) stack_size;

    uint32_t * stack = os_stack_get(thread->stack_id);
    thread->sp = stack; //&stack[stack_size - 16];

    int rv = pipe(thread->arch.block_pipe);
    assert(rv == 0);

    struct thread_startup_t * startup_data = malloc(sizeof(struct thread_startup_t));
    memset(startup_data, 0, sizeof(struct thread_startup_t));
    startup_data->thread_id = thread - os_threads;
    startup_data->entry_point = entrypoint;
    startup_data->entry_arg = data;
    rv = thrd_create(&thread->arch.sched_thread, &thread_startup_handler, startup_data);
    // Synchronize with newly-created thread, so that POSIX thread that hosts it
    // is already suspended before we continue
    while (!thread->arch.is_suspended) {}
    assert(rv == thrd_success);
}

/** Platform-specific way of initializing threads.
 * Nothing is done here. No special facilities for
 * processes in this port.
 */
int os_process_create(Process_t process_id, const struct OS_process_definition_t * definition)
{
    (void) process_id;
    (void) definition;
    if (process_id >= OS_PROCESSES)
    {
        return E_OUT_OF_RANGE;
    }

    if (os_processes[process_id].definition != NULL)
    {
        return E_INVALID;
    }

    /* TODO: Deal with memory protection */

    os_processes[process_id].definition = definition;
    return E_OK;
}

/** Will unblock the boot thread.
 *
 * Unblock the boot thread. Then this function continues listening for
 * signals for timer and thread switching requests.
 */
__attribute__((noreturn)) void os_boot_thread(Thread_t boot_thread)
{
    char byte = 0;

    write(os_threads[boot_thread].arch.block_pipe[1], &byte, 1);

    while (1)
    {
        /* This is the main thread, we have to keep it running.
         */
        sleep(1);
    }
}

/** Set/clear thread switch request flag.
 * @param activate new value of the flag
 */
void os_request_context_switch(bool activate)
{
    kernel_execute_thread_switch = activate;

}

int os_set_syscall_return_value(Thread_t thread_id, int32_t retval)
{
    os_threads[thread_id].arch.syscall.retval = retval;
    return 0;
}

void os_memory_protection_start()
{
    /* TODO */
}

void os_memory_protection_stop()
{
    /* TODO */
}

int mpu_restore(const MPU_State * hosted_state, const MPU_State * parent_state)
{
    /* TODO */
    return 0;
}

int mpu_init_stack(int thread_id)
{
    /* TODO */
    return 0;
}

__attribute__((noreturn)) void os_kernel_shutdown()
{
    exit(0);
}

int os_rpc_call(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    (void) arg0;
    (void) arg1;
    (void) arg2;
    (void) arg3;

    int thread = os_get_current_thread();
    struct syscall_dispatch_t * syscall = &os_threads[thread].arch.syscall;

    RPC_Service_t * service = (RPC_Service_t *) syscall->args[4];
    VTable_t vtable = service->vtable;
#ifdef __APPLE__
    /* TODO: Vtable regions are not yet implemented on MacOS */
    Process_t process_id = 0; //get_vtable_process(vtable);
#else
    Process_t process_id = get_vtable_process(vtable);
#endif

    if (process_id == E_VTABLE_UNKNOWN)
    {
        return E_INVALID_ADDRESS;
    }

    if (!rpc_stack_push(process_id))
    {
        return E_IN_TOO_DEEP;
    }

    unsigned method_id = syscall->args[5];
    RPC_Method_t method = vtable[method_id];

    syscall->outcome = SYSCALL_OUTCOME_RPC_CALL;
    syscall->dispatch_target = method;
    for (int q = 0; q < 4; ++q)
    {
        syscall->dispatch_args[q+1] = syscall->args[q];
    }
    syscall->dispatch_args[0] = (unsigned long) service;

    return E_OK;
}

int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    (void) arg0;
    (void) arg1;
    (void) arg2;
    (void) arg3;

    int pstack_depth = rpc_stack_pop();
    Process_t process_id;

    if (pstack_depth > 0)
    {
        process_id = rpc_stack_top();
    }
    else
    {
        /* Warning for future wanderers: as of now, this returns
         * process_id of current thread, which stores parent process.
         * If I ever decide to change semantics to return current process
         * ID, this may fail miserably.
         */
        process_id = os_get_current_process();
    }


    if (process_id == E_VTABLE_UNKNOWN)
    {
        // here the process should probably die in segfault
        assert(0);
    }

    return arg0;
}

struct Syscall_Entry_t * os_syscalls_start(void)
{
    return syscalls;
}

struct Syscall_Entry_t * os_syscalls_end(void)
{
    return &syscalls[syscalls_count];
}

void os_core_sleep()
{
    sleep(1);
}
