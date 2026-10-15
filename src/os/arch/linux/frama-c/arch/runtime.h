#pragma once

#include <kernel/rpc.h>

#define os_init_core(x)

struct OS_thread_t;

/*@
    requires \valid(thread) && entrypoint != \null;
    assigns \nothing;
 */
inline void os_thread_initialize_arch(struct OS_thread_t * thread, unsigned stack_size, entrypoint_t * entrypoint, void * data)
{
}

/*@
    assigns \nothing;
*/
inline void os_init_arch(void)
{
}

/** The outcome of the system call
 */
enum SyscallOutcome {
    /// Return normally
    SYSCALL_OUTCOME_RETURN,
    /// Call some specific code
    SYSCALL_OUTCOME_RPC_CALL
};

/** Syscall dispatch data structure
 *
 * Internal structure to dispatch arguments to
 * syscall and return system call return value.
 */
struct syscall_dispatch_t {
    long args[6];
    long retval;
    unsigned char syscall_id;
    enum SyscallOutcome outcome;
    RPC_Method_t dispatch_target;
    long dispatch_args[5];
};

/** Linux port internal architecture state.
 *
 * This structure records thread state private to the Linux port.
 * Here blocking primitives for synchronization of threads and
 * system calls are recorded as well as block where information
 * is passed down to the syscall and back from it.
 */
struct Arch_State_t {
    int foo;
};

