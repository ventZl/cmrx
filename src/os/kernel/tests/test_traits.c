#include "test_traits.h"

#include <stdlib.h>

enum ThreadState thread_state(uint8_t thread)
{
    if (thread > OS_THREADS)
    {
        abort();
    }
    return os_threads[thread].state;
}
