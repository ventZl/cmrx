/** @ingroup os_syscall
 * @{
 */

#pragma once

/** @defgroup os_syscalls List of system call IDs
 *
 * Due to limitations of ELF/gcc/linker, system call IDs must be known
 * at compile time as they are not relocatable. This list contains 
 * assigned IDs of system calls.
 *
 * @{
 */
#define SYSCALL_GET_TID						0
#define SYSCALL_SCHED_YIELD					1
#define SYSCALL_THREAD_CREATE				2
#define SYSCALL_MUTEX_INIT					3
#define SYSCALL_MUTEX_DESTROY				4
#define SYSCALL_MUTEX_UNLOCK				5
#define SYSCALL_MUTEX_TRYLOCK				6
#define SYSCALL_RPC_CALL					7
#define SYSCALL_RPC_RETURN					8
#define SYSCALL_THREAD_JOIN					9
#define SYSCALL_THREAD_EXIT					10
#define SYSCALL_SETITIMER					11
#define SYSCALL_USLEEP						12
#define SYSCALL_SIGNAL						13
#define SYSCALL_KILL						14
#define SYSCALL_SETPRIORITY					15

/** @} */
/** @} */
