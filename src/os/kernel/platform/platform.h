#pragma once

/* Prototype for platform-specific system call (optional)
 * Your platform is free to define function having this name and prototype. If it does,
 * then this version will override kernel-default version that does nothing. You can
 * place platform-specific hooks which need kernel privileges to execute here.
 *
 * If your platform doesn't need any such feature, then don't include this function in
 * your platform code, the default version will be used which provides no functionality.
 * @param arg1 system call argument 1
 * @param arg2 system call argument 2
 * @param arg3 system call argument 3
 * @param arg4 system call argument 4
 * @returns outcome of the system call, it will be made available to the calling thread
 * when system call returns.
 **/
int os_sys_platform_call(uint32_t arg1, uint32_t arg2, uint32_t arg3, uint32_t arg4);

/** Prototype for platform-specific CPU core initialization (optional)
 * Your platform may contain some core-specific initialization code that needs to be
 * executed on each and every CPU core. You can define function with this name and it
 * will be automatically called.
 *
 * If your platform doesn't need any such functionality, then use empty macro below to
 * deactivate any calls to the function.
 */
void os_init_platform(void);

#define os_init_platform()

/** Prototype for platform-specific thread initialization (optional)
 * Your platform may contain some specific code that needs to be run on every thread
 * creation. It can be placed into function with this name and it will be automatically
 * issued after architecture has initialized thread details.
 *
 * If your platform doesn't need any such functionality, then use empty macro below to
 * deactivate any calls to the function.
 * @param thread TCB of thread just created
 */
void os_thread_initialize_platform(struct OS_thread_t * thread);
