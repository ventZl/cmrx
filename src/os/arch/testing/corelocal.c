#include <arch/corelocal.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

unsigned cmrx_current_core = 0;
unsigned cmrx_os_smp_locked = 0;

unsigned cmrx_os_core_lock_called = 0;
unsigned cmrx_os_core_unlock_called = 0;

callback_t cmrx_smp_locked_callback = NULL;
callback_t cmrx_smp_unlocked_callback = NULL;
callback_t cmrx_smp_wrong_lock_callback = NULL;

unsigned coreid() 
{ 
    return cmrx_current_core; 
}

void os_core_lock() {
    cmrx_os_core_lock_called = 1;
}

void os_core_unlock() {
    cmrx_os_core_unlock_called = 1;
}


void os_smp_lock() { 
    if (cmrx_os_smp_locked)
    {
        if (cmrx_smp_wrong_lock_callback) {
            cmrx_smp_wrong_lock_callback();
            return;
        } else {
            printf("\nFATAL ERROR: Attempt to lock BKL while already locked!\n");
            // Attempt to lock recursively!
            abort();
        }
    }
//    printf("\nBKL lock!");
    cmrx_os_smp_locked = 1; 
    if (cmrx_smp_locked_callback != NULL) 
    {
        cmrx_smp_locked_callback(); 
    }
}

void os_smp_unlock() { 
    if (!cmrx_os_smp_locked)
    {
        if (cmrx_smp_wrong_lock_callback) {
            cmrx_smp_wrong_lock_callback();
            return;
        } else {
            printf("\nFATAL ERROR: Attempt to unlock BKL while not locked!\n");
            // Attempt to unlock when no lock is locked!
            abort();
        }
    }
//    printf("\nBKL unlock!");
    cmrx_os_smp_locked = 0; 
    if (cmrx_smp_unlocked_callback != NULL)
    {
        cmrx_smp_unlocked_callback();
    }
}
