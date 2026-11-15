#pragma once

/** @addtogroup arch_arch
 * @{
 */

/** Platform-specific system call entrypoint (optional)
 * Use as very last resort measure! This allows running platform-specific code in kernel context.
 */
int os_sys_platform_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/** @} */
