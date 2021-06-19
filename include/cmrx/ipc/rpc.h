/** @defgroup api_rpc Remote Procedure Calls
 *
 * @ingroup api
 */
#pragma once

#include <cmrx/os/sysenter.h>
#include <stddef.h>

#define RPC_GET_ARG_COUNT_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...)	N
#define RPC_GET_ARG_COUNT(...)			RPC_GET_ARG_COUNT_HELPER(__VA_ARGS__ __VA_OPT__(,) 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define RPC_PASTER(argcount)	RPC_CALL_ ## argcount
#define RPC_EVALUATOR(argcount)	RPC_PASTER(argcount)
#define RPC_CALL_4(si, mi, _0, _1, _2, _3)	_rpc_call((unsigned) _0, (unsigned) _1, (unsigned) _2, (unsigned) _3, si, mi, 0xAA55AA55)
#define RPC_CALL_3(si, mi, _0, _1, _2)		_rpc_call((unsigned) _0, (unsigned) _1, (unsigned) _2, 0, si, mi, 0xAA55AA55)
#define RPC_CALL_2(si, mi, _0, _1)			_rpc_call((unsigned) _0, (unsigned) _1, 0, 0, si, mi, 0xAA55AA55)
#define RPC_CALL_1(si, mi, _0)				_rpc_call((unsigned) _0, 0, 0, 0, si, mi, 0xAA55AA55)
#define RPC_CALL_0(si, mi)					_rpc_call(0, 0, 0, 0, si, mi, 0xAA55AA55)

/**
 * @ingroup api_rpc
 * @{
 */

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
	RPC_EVALUATOR(RPC_GET_ARG_COUNT(__VA_ARGS__))(\
			(service_instance), \
			offsetof(typeof(*((service_instance)->vtable)), method_name) / sizeof(void *), \
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
__SYSCALL int _rpc_call(unsigned arg0, unsigned arg1, unsigned arg2, unsigned artg3, void * service, unsigned method, unsigned canary);

/** The way how RPC returns. Used automatically.
 *
 * Kernel uses this to return from RPC. It is hooked into RPC call chain automatically, no need
 * to call it manually from RPC method. It is sufficient to return from RPC to call this.
 */
__SYSCALL void rpc_return();

/** @} */
