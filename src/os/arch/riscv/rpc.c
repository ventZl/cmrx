#include <kernel/rpc.h>
#include <cmrx/defines.h>

int os_rpc_call(unsigned long arg0, unsigned long arg1, unsigned long arg2, unsigned long arg3)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return E_NOTAVAIL;
}

int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3)
{
    (void)arg0;
    (void)arg1;
    (void)arg2;
    (void)arg3;
    return E_NOTAVAIL;
}
