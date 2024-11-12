#include <conf/kernel.h>
#include <arch/mpu.h>
#include <stdbool.h>

bool memory_protection_stop_called = false;
bool kernel_shutdown_called = false;

void os_memory_protection_start()
{

}

void os_memory_protection_stop()
{
    memory_protection_stop_called = true;
}

__attribute__((weak)) void cmrx_shutdown_handler()
{

}

void os_kernel_shutdown()
{
    kernel_shutdown_called = true;
    cmrx_shutdown_handler();
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
