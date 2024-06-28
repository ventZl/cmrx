#pragma once

/** @addtogroup arch_arch
 * @{ 
 */

/** Provide kernel with identification of current CPU core
 *
 * Kernel calls this function if it is configured for SMP. This function
 * is part of the porting layer but does not need to be implemented in 
 * port. If the architecture does not provide unified way of determining
 * which core is the one currently executing this code, then the implementation
 * of this method may be left to the user.
 * @returns ID of the currently running code
 */
unsigned coreid();

/** Lock the "big kernel lock"
 *
 * This function starts the critical section within kernel. Actions that happen
 * within critical section can ever only happen on one CPU core and other cores
 * must not motify the same data. Big kernel lock is used to lock thread table,
 * stack allocation table and sleepers table.
 *
 * This function is part of porting layer but depends heavily on target CPU. It
 * may be left to be implemented by the user. If this function is called, the code
 * should make sure that if this function is called on any other core before
 * @ref os_smp_unlock() is called that another call won't proceed.
 */
void os_smp_lock();

/** Unlock the "big kernel lock"
 *
 * This function ends the critical setion within kernel. 
 * 
 * This function is part of the porting layer but depends heavily on target CPU.
 * It may be left the be implemented by the user. Once this function is called
 * the code must make sure that any other core potentially being blocked by 
 * parallel call to @ref os_smp_lock() will be unlocked and continue execution.
 */
void os_smp_unlock();


