#include <stdbool.h>
#include <kernel/arch/context.h>
#include <cmrx/arch/riscv/context_switch.h>

void os_request_context_switch(bool activate)
{
	os_riscv_context_switch_request(activate);
}
