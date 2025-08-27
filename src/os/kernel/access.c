#include "access.h"
#include <conf/kernel.h>
#include <cmrx/assert.h>

#include "runtime.h"

bool os_thread_check_access(uint8_t thread_id, void * address, int access_type)
{
    (void) thread_id;
    (void) address;
    (void) access_type;
    return true;
}
