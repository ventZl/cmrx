#pragma once

#include <cmrx/os/sysenter.h>
#include <stddef.h>

#define rpc_call(service_instance, method_name, ...) \
	_rpc_call(\
			(service_instance), \
			offsetof(typeof(*((service_instance)->vtable)), method_name) / sizeof(void *),\
			##__VA_ARGS__)

int _rpc_call(void * service, unsigned method, ...);

__SYSCALL void rpc_return();
