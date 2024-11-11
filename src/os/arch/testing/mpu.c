#include <conf/kernel.h>
#include <arch/mpu.h>

void os_memory_protection_start()
{

}

void os_memory_protection_stop()
{

}

void os_kernel_shutdown()
{

}

int mpu_init_stack(int thread_id)
{
    (void) thread_id;
    return 0;
}

int mpu_restore(const MPU_State * hosted_state, const MPU_State * parent_state)
{
    (void) hosted_state;
    (void) parent_state;
	return 0;
}
