#pragma once

/** Method to push signal handler onto thread stack.
 * Modifies threads stack and registers to continue executing signal handler.
 * @param thread thread whose signal handler should be pushed onto stack
 * @param signals signal mask of signals being delivered
 */
void os_deliver_signal(struct OS_thread_t * thread, uint32_t signals);
