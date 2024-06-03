/** @defgroup arch_arm_syscall System calls implementation
 * @ingroup arch_arm
 * @{
 */
#include <stdint.h>
#include <cmrx/os/syscall.h>

#include <cmrx/os/sched.h>
#include <cmrx/os/sanitize.h>

#include <arch/sysenter.h>
#include <arch/cortex.h>

/** ARM-specific entrypoint for system call handlers.
 *
 * This routine is common entrypoint for all syscall routines. It decodes
 * the syscall ID requested by userspace application and calls generic
 * method to service the system call. It is callable by executing the SVC 
 * instruction. Code of SVC_Handler will retrieve the requested SVC ID and
 * let generic machinery to execute specified system call.
 * @param arg0 syscall argument
 * @param arg1 syscall argument
 * @param arg2 syscall argument
 * @param arg3 syscall argument
 */
__attribute__((interrupt)) void SVC_Handler(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
	uint32_t * psp = (uint32_t *) __get_PSP();
	sanitize_psp(psp);
	uint16_t * lra = (uint16_t *) *(psp + 6);
	uint8_t syscall_id = *(lra - 1);
    uint32_t rv = os_system_call(arg0, arg1, arg2, arg3, syscall_id);
    *(psp) = rv;
    return; /*asm volatile("BX lr");*/
}

/** @} */
