#pragma once

#define rpc_call(service_instance, method_name, arg0, arg1, arg2, arg3) \
	_rpc_call((service_instance), ((void **) &((service_instance)->vtable->method_name) - (void **) ((service_instance)->vtable)), arg0, arg1, arg2, arg3)

int _rpc_call(void * service, unsigned method, unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3);


