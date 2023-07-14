#pragma once

#include <cmrx/rpc/interface.h>
#include <stdint.h>

struct ServiceVTable {
    uint32_t (*method)(INSTANCE(this), uint32_t arg1);
};

struct Service {
    const struct ServiceVTable * vtable;
    uint32_t value;
};

extern struct Service * service_ptr;



