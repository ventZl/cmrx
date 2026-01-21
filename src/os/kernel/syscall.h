#pragma once

#include <stdbool.h>
#include <stdint.h>

/** @defgroup os_syscall System calls
 *
 * @ingroup os
 *
 * Kernel contains mechanism of using kernel services. This mechanism (commonly
 * known as system calls, or syscalls) can be used to call system services only
 * from userspace code running in thread context. It is not possible to call these
 * services from within ISR context.
 */

/** @ingroup os_syscall
 * @{
 */

/** Attribute used to publish syscall definitions in a way that
 * os_syscall can process them
 */
#define SYSCALL_DEFINITION __attribute__((section(".syscall"), used)) const

typedef int (* Syscall_Handler_t)(unsigned long, unsigned long, unsigned long, unsigned long);

/** Entry in syscall table.
 *
 * This entry pairs syscall number with syscall handler function.
 */
struct Syscall_Entry_t {
	/** Syscall ID. Cortex-M supports 255 syscalls.
	 */
	uint8_t id;

	/** Address of handler function.
	 */
	Syscall_Handler_t handler;
};

int os_system_call_init(void);

/** Find and execute a system call.
 * 
 * Execution of system call is a portable action. It translates into calling a function
 * determined by checking the syscall table. This function will call the syscall and return
 * the return value it provided.
 * @param arg0 1st argument to the syscall routine
 * @param arg1 2nd argument to the syscall routine
 * @param arg2 3rd argument to the syscall routine
 * @param arg3 4th argument to the syscall routine
 * @param syscall_id the ID of system call routine that has to be called
 * @returns value provided by the system call routine, if routine with given syscall_id exists.
 * Otherwise returns E_NOTAVAIL.
 */
int os_system_call(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3, uint8_t syscall_id);

/** @} */
