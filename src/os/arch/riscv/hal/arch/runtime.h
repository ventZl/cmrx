#pragma once

/* No special handling yet; platform integration will define this as needed. */
#define os_thread_initialize_arch(...)

struct Arch_State_t {
    /* intentionally left empty */
};

#define os_init_arch(x)
#define os_init_core(x)

