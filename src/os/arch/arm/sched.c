/** @defgroup arch_arm_sched Scheduler implementation
 *
 * @ingroup arch_arm
 *
 * @brief Kernel scheduler internals
 *
 * @{
 */
#include <kernel/runtime.h>
#include <kernel/sched.h>
#include <kernel/timer.h>
#include <kernel/arch/sched.h>
#include <arch/memory.h>
#include <arch/cortex.h>
#include <kernel/mpu.h>
#include <arch/mpu.h>
#include <arch/mpu_priv.h>
#include <string.h>

#ifdef TESTING
#define STATIC
#else
#define STATIC static
#endif


#include <cmrx/assert.h>

/** Populate stack of new thread so it can be executed.
 * Populates stack of new thread so that it can be executed with no
 * other actions required. Returns the address where SP shall point to.
 * @param stack_id ID of stack to be populated
 * @param stack_size size of stack in 32-bit quantities
 * @param entrypoint address of thread entrypoint function
 * @param data address of data passed to the thread as its 1st argument
 * @returns Address to which the SP shall be set.
 */
uint32_t * os_thread_populate_stack(int stack_id, unsigned stack_size, entrypoint_t * entrypoint, void * data)
{
    uint32_t * stack = os_stack_get(stack_id);
    stack[stack_size - 8] = (unsigned long) data; // R0
    stack[stack_size - 3] = (unsigned long) os_thread_dispose; // LR
    stack[stack_size - 2] = (unsigned long) entrypoint; // PC
    stack[stack_size - 1] = 0x01000000; // xPSR

    return &stack[stack_size - 16];

}

int os_process_create(Process_t process_id, const struct OS_process_definition_t * definition)
{
	if (process_id >= OS_PROCESSES)
	{
		return E_OUT_OF_RANGE;
	}
	
	if (os_processes[process_id].definition != NULL)
	{
		return E_INVALID;
	}

	os_processes[process_id].definition = definition;
	for (int q = 0; q < OS_TASK_MPU_REGIONS; ++q)
	{
		unsigned reg_size = (uint8_t *) definition->mpu_regions[q].end - (uint8_t *) definition->mpu_regions[q].start;
		int rv = mpu_configure_region(q, definition->mpu_regions[q].start, reg_size, MPU_RW, &os_processes[process_id].mpu[q]._MPU_RBAR, &os_processes[process_id].mpu[q]._MPU_RASR);
		if (rv != E_OK)
		{
			os_processes[process_id].definition = NULL;
			return E_INVALID;
		}
	}
	return E_OK;
}

/// @cond IGNORE
__attribute__((noreturn))
/// @endcond
void os_boot_thread(Thread_t boot_thread)
{
    // Start this thread
    // We are adding 8 here, because normally pend_sv_handler would be reading 8 general 
    // purpose registers here. But there is nothing useful there, so we simply skip it.
    // Code belog then restores what would normally be restored by return from handler.
    struct OS_thread_t * thread = os_thread_get(boot_thread);
    uint32_t * thread_sp = thread->sp + 8;
    __set_PSP((uint32_t) thread_sp);
    __set_CONTROL(0x03); 	// SPSEL = 1 | nPRIV = 1: use PSP and unpriveldged thread mode

    __ISB();

    __ISR_return();

	while (1);
}

__attribute__((noreturn)) void os_reset_cpu()
{
	SCB->AIRCR = (0x5FA<<SCB_AIRCR_VECTKEY_Pos)|SCB_AIRCR_SYSRESETREQ_Msk;
	while (1);
}


/** Default CMRX shutdown handler for ARM CPUs
 * This is default action after for when the CMRX kernel is shutdown.
 * It will reset the CPU. If you want to perform different action, then
 * provide your function called `cmrx_shutdown_handler. It will override
 * this one.
 * @note Your function should not return nor try to restart CMRX kernel.
 * If you want to restart the kernel, then perform CPU reset first.
 */
__attribute__((weak)) void cmrx_shutdown_handler(void)
{
	os_reset_cpu();
}

/** Perform the kernel shutdown.
 * This is platform-specific way of how to shutdown the kernel. In this case
 * an interrupt frame is forged on stack that will resemble a frame returning
 * back to the @ref cmrx_shutdown_handler function. The result of running
 * this function will be that the processor leaves the handler mode, enters
 * privileged thread mode and will be using MSP.
 */
/// @cond IGNORE
__attribute__((noreturn))
/// @endcond
void os_kernel_shutdown()
{
	__set_CONTROL(0); // SPSEL = 0 | nPRIV = 0; use MSP and privileged thread mode

	__forge_shutdown_exception_frame(cmrx_shutdown_handler);
}


int os_set_syscall_return_value(Thread_t thread_id, int32_t retval)
{
    struct OS_thread_t * thread = os_thread_get(thread_id);
	if (thread == NULL)
	{
		return E_INVALID;
	}

	ExceptionFrame * ef = (ExceptionFrame *) thread->sp;
	ef->r0123[0] = retval;
	return E_OK;
}

/** @} */
