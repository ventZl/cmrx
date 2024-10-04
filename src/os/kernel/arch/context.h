#pragma once

/** @addtogroup arch_arch
 * @{ 
 */

/** Request context switch.
 * This function is called by the platform independent part
 * of the kernel when @ref cpu_context structure is filled 
 * with valid data and context switch shall happen.
 * The implementation of this function should configure the
 * hardware in a way that context switch will happen as soon
 * as kernel finishes its work and is ready to return the
 * CPU back to the userspace code.
 */
void os_request_context_switch();
