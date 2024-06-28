#include <arch/corelocal.h>
#include <stddef.h>
#include <stdio.h>

unsigned cmrx_current_core = 0;
unsigned cmrx_os_smp_locked = 0;

callback_t cmrx_smp_locked_callback = NULL;
callback_t cmrx_smp_unlocked_callback = NULL;

unsigned coreid() 
{ 
    return cmrx_current_core; 
}

void os_smp_lock() { 
    cmrx_os_smp_locked = 1; 
    if (cmrx_smp_locked_callback != NULL) 
    {
        cmrx_smp_locked_callback(); 
    }
}

void os_smp_unlock() { 
    cmrx_os_smp_locked = 0; 
    if (cmrx_smp_unlocked_callback != NULL)
    {
        cmrx_smp_unlocked_callback();
    }
}
