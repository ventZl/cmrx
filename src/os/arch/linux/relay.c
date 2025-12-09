#include "relay.h"
#include <cmrx/ipc/thread.h>

/* Routines that call CMRX standard library */

void linux_thread_exit(int status)
{
    thread_exit(status);
}
