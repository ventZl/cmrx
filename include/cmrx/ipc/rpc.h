#pragma once

#include <cmrx/os/sysenter.h>
#include <stddef.h>

/** User-visible way to perform remote procedure call.
 *
 * Calls service published by different thread and/or process. Service can take 0 to 4
 * arguments, which are passed down to it. Return value of service is passed back to the
 * caller.
 * @param service_instance address of service instance, which is being called
 * @param method_name name of method within service, which has to be called
 * @returns whatever value service returned
 */
#define rpc_call(service_instance, method_name, ...) \
	_rpc_call(\
			(service_instance), \
			offsetof(typeof(*((service_instance)->vtable)), method_name) / sizeof(void *),\
			##__VA_ARGS__)

/** Internal implementation of remote procedure call in userspace.
 *
 * This function is actually called when user calles @ref rpc_call(). The only difference is,
 * that this signature takes method number instead it's name. Macro rpc_call will figure this
 * out automatically.
 *
 * @param service address of service instance
 * @param method offset of method in VMT of service
 * @return whatever service method returns
 */
int _rpc_call(void * service, unsigned method, ...);

/** The way how RPC returns. Used automatically.
 *
 * Kernel uses this to return from RPC. It is hooked into RPC call chain automatically, no need
 * to call it manually from RPC method. It is sufficient to return from RPC to call this.
 */
__SYSCALL void rpc_return();
