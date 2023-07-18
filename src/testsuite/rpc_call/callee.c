#include <cmrx/application.h>
#include <cmrx/ipc/thread.h>
#include <debug.h>
#include <cmrx/rpc/interface.h>
#include "service.h"

#include <cmrx/rpc/implementation.h>

IMPLEMENTATION_OF(struct Service, struct ServiceVTable);

uint32_t service_method(INSTANCE(this), uint32_t arg1)
{
    this->value = arg1;
    TEST_SUCCESS();
    return arg1;
}

VTABLE struct ServiceVTable service_vtable = {
    service_method
};

struct Service service = {
    &service_vtable,
    0
};

OS_APPLICATION_MMIO_RANGE(rpc_call_callee, 0x40000000, 0x60000000);
OS_APPLICATION(rpc_call_callee);

