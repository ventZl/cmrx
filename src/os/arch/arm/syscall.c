/** @defgroup arch_arm_syscall System calls implementation
 * @ingroup arch_arm
 * @{
 */
#include <stdint.h>
#include <cmrx/sys/syscalls.h>
#include <cmrx/assert.h>
#include <kernel/syscall.h>

#include <kernel/sched.h>
#include <kernel/sanitize.h>

#include <arch/sysenter.h>
#include <arch/cortex.h>
#include <arch/nvic.h>

#include <kernel/trace.h>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wcast-function-type"

static SYSCALL_DEFINITION struct Syscall_Entry_t nvic_syscalls[] = {
	{ SYSCALL_ENABLE_IRQ, (Syscall_Handler_t) &os_nvic_enable },
	{ SYSCALL_DISABLE_IRQ, (Syscall_Handler_t) &os_nvic_disable },
};

#pragma GCC diagnostic pop


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
__attribute__((interrupt)) void SVC_Handler(void)
{
	// This assert checks that we are not preempting some other interrupt
	// handler. If you assert here, then your interrupt handler priority
	// is messed up. You need to configure PendSV to be the handler with
	// absolutely the lowest priority.
	ASSERT(__get_LR() == (void *) 0xFFFFFFFDU);

	uint32_t * psp = (uint32_t *) __get_PSP();
	sanitize_psp(psp);
	ExceptionFrame * frame = (ExceptionFrame *) psp;
	uint16_t * lra = (uint16_t *) frame->pc;
	uint8_t syscall_id = *(lra - 1);
	trace_event(EVENT_SYSCALL, syscall_id);
    uint32_t rv = os_system_call(frame->r0123[0], frame->r0123[1], frame->r0123[2], frame->r0123[3], syscall_id);
    *(psp) = rv;
    return; /*asm volatile("BX lr");*/
}

/** @} */
