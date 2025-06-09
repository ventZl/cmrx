#include <kernel/arch/mpu.h>
#include <kernel/arch/sched.h>
#include <kernel/arch/context.h>
#include <kernel/syscall.h>
#include <kernel/sched.h>
#include <conf/kernel.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

ucontext_t kernel_context;
volatile bool kernel_execute_thread_switch = false;

void trigger_pendsv_if_needed()
{
    if (kernel_execute_thread_switch)
    {
        raise(SIGUSR1);
    }
}

int system_call_entrypoint(unsigned long arg0,
                            unsigned long arg1,
                            unsigned long arg2,
                            unsigned long arg3,
                            unsigned long arg4,
                            unsigned long arg5,
                            unsigned char syscall_id)
{
    int rv = os_system_call(arg0, arg1, arg2, arg3, syscall_id);
    trigger_pendsv_if_needed();
    return rv;
}

void PendSV_Handler(int signo, siginfo_t *info, void *context)
{
    struct OS_core_state_t * cpu_state = &core[coreid()];

    if (os_threads[cpu_state->thread_current].state == THREAD_STATE_RUNNING)
    {
        // only mark leaving thread as ready, if it was runnig before
        // if leaving thread was, for example, quit before calling
        // os_sched_yield, then this would return it back to life
        os_threads[cpu_state->thread_current].state = THREAD_STATE_READY;
    }

    	cpu_state->thread_current = cpu_state->thread_next;

        os_threads[cpu_state->thread_current].state = THREAD_STATE_RUNNING;

    printf("Switching into thread %d...", cpu_state->thread_current);
    int rv = setcontext(&(os_threads[cpu_state->thread_current].arch.thread_context));
    printf("%d\n", rv);
}

void os_init_arch(void)
{
    getcontext(&kernel_context);

    struct sigaction act = { 0 };
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &PendSV_Handler;

    sigaction(SIGUSR1, &act, NULL);
}

void os_thread_initialize_arch(struct OS_thread_t * thread, unsigned stack_size, entrypoint_t * entrypoint, void * data)
{
    uint32_t * stack = os_stack_get(thread->stack_id);
    thread->sp = stack; //&stack[stack_size - 16];

    getcontext(&thread->arch.thread_context);
    thread->arch.thread_context.uc_link = &kernel_context;
    thread->arch.thread_context.uc_stack.ss_sp = thread->sp;
    thread->arch.thread_context.uc_stack.ss_size = OS_STACK_SIZE;
    makecontext(&thread->arch.thread_context, (void (*)()) entrypoint, 1, data);
}

int os_process_create(Process_t process_id, const struct OS_process_definition_t * definition)
{
    return 0;
}

__attribute__((noreturn)) void os_boot_thread(Thread_t boot_thread)
{
    setcontext(&(os_threads[boot_thread].arch.thread_context));
    while (1)
    {
        sleep(1);
        printf("Wadda wadda\n");
    }
}

void os_request_context_switch(bool activate)
{
    kernel_execute_thread_switch = activate;

}

int os_set_syscall_return_value(Thread_t thread_id, int32_t retval)
{
    return 0;
}

void os_memory_protection_start()
{
    /* This function intentionally left blank */
}

void os_memory_protection_stop()
{
    /* This function intentionally left blank */
}

int mpu_restore(const MPU_State * hosted_state, const MPU_State * parent_state)
{
    /* This function intentionally left blank */
    return 0;
}

int mpu_init_stack(int thread_id)
{
    /* This function intentionally left blank */
    return 0;
}

__attribute__((noreturn)) void os_kernel_shutdown()
{
    exit(0);
}

int os_rpc_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    return E_NOTAVAIL;
}

int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    return E_NOTAVAIL;
}

static struct Syscall_Entry_t syscalls[256] = { 0 };
static unsigned syscalls_count = 0;

void cmrx_posix_register_syscalls(struct Syscall_Entry_t * added_syscalls)
{
    while (added_syscalls->handler != 0 && added_syscalls->id != 0 )
    {
        syscalls[syscalls_count++] = *added_syscalls;
    }
}

struct Syscall_Entry_t * os_syscalls_start(void)
{
    return syscalls;
}

struct Syscall_Entry_t * os_syscalls_end(void)
{
    return &syscalls[syscalls_count];
}
