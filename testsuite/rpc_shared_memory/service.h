#pragma once

#include <cmrx/rpc/interface.h>
#include <stdint.h>

struct ServiceVTable {
    uint32_t (*method)(INSTANCE(this), uint32_t arg1, const char * buffer);
};

struct Service {
    const struct ServiceVTable * vtable;
    char value;
};

extern struct Service service;



