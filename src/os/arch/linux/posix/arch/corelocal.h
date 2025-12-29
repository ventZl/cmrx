#pragma once

#define __todo__        _Static_assert(1, "This needs to be filled-in")

#define os_core_lock()      __todo__
#define os_core_unlock()    __todo__

// We don't support SMP yet, so these are no-op

#define os_smp_lock()
#define os_smp_unlock()

#define coreid() 0

#define OS_NUM_CORES	1

void os_core_sleep(void);
