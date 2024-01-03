/** @defgroup arch_arm_sched Scheduler implementation
 *
 * @ingroup arch_arm
 *
 * @brief Kernel scheduler internals
 *
 * @{
 */
#include <cmrx/os/runtime.h>
#include <cmrx/os/sched.h>
#include <cmrx/os/timer.h>
#include <cmrx/os/arch/sched.h>
#include <arch/memory.h>
#include <arch/cortex.h>
#include <cmrx/os/mpu.h>
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
    unsigned long * stack = os_stack_get(stack_id);
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
__attribute__((naked,noreturn)) 
/// @endcond
void os_boot_thread(Thread_t boot_thread)
{
    // Start this thread
    // We are adding 8 here, because normally pend_sv_handler would be reading 8 general 
    // purpose registers here. But there is nothing useful there, so we simply skip it.
    // Code belog then restores what would normally be restored by return from handler.
    struct OS_thread_t * thread = os_thread_get(boot_thread);
    unsigned long * thread_sp = thread->sp + 8;
    __set_PSP((uint32_t) thread_sp);
    __set_CONTROL(0x03); 	// SPSEL = 1 | nPRIV = 1: use PSP and unpriveldged thread mode

    __ISB();

    __ISR_return();

}

/** @} */
