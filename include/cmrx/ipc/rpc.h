/** @defgroup api_rpc Remote Procedure Calls
 *
 * @ingroup api
 */
#pragma once

#include <cmrx/os/sysenter.h>
#include <stddef.h>

/*
 * Rearrange arguments to rpc_call syscall.
 * We want to move RPC call arguments to occupy argument 1 .. 4 position.
 * service and method goes next. The macro magic below deals with varying amount
 * of arguments so that all calls to rpc_call() with less than 4 RPC arguments are
 * padded to actual 4 arguments.
 */

#define CMRX_RPC_GET_ARG_COUNT_HELPER(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...)	N
#define CMRX_RPC_GET_ARG_COUNT(...)			CMRX_RPC_GET_ARG_COUNT_HELPER(__VA_ARGS__ __VA_OPT__(,) 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)

#define CMRX_RPC_PASTER(argcount)	            CMRX_RPC_CALL_ ## argcount
#define CMRX_RPC_EVALUATOR(argcount)	        CMRX_RPC_PASTER(argcount)
#define CMRX_RPC_CALL_4(si, mi, _0, _1, _2, _3)	_rpc_call((unsigned) _0, (unsigned) _1, (unsigned) _2, (unsigned) _3, si, mi, 0xAA55AA55)
#define CMRX_RPC_CALL_3(si, mi, _0, _1, _2)		_rpc_call((unsigned) _0, (unsigned) _1, (unsigned) _2, 0, si, mi, 0xAA55AA55)
#define CMRX_RPC_CALL_2(si, mi, _0, _1)			_rpc_call((unsigned) _0, (unsigned) _1, 0, 0, si, mi, 0xAA55AA55)
#define CMRX_RPC_CALL_1(si, mi, _0)				_rpc_call((unsigned) _0, 0, 0, 0, si, mi, 0xAA55AA55)
#define CMRX_RPC_CALL_0(si, mi)					_rpc_call(0, 0, 0, 0, si, mi, 0xAA55AA55)

/*
 * Perform compile time type checking of the RPC call arguments.
 * This will emit code which apparently calls the rpc method directly. This call
 * is never made in the runtime (and is optimized out as dead code anyway) but it will
 * emit a fairly readable error if types of arguments to the RPC call don't match.
 */ 

#define CMRX_RPC_TYPE_CHECKER(argcount, object, ...)      if (0) { object((void *) 0 __VA_OPT__(,) __VA_ARGS__); }

#define CMRX_RPC_CHECK_TYPE_4(object, _0, _1, _2, _3)	CMRX_RPC_CHECK_TYPE(object, (_0, _1, _2, _3))
#define CMRX_RPC_CHECK_TYPE_3(object, _0, _1, _2)		CMRX_RPC_CHECK_TYPE(object, (_0, _1, _2))
#define CMRX_RPC_CHECK_TYPE_2(object, _0, _1)			CMRX_RPC_CHECK_TYPE(object, (_0, _1))
#define CMRX_RPC_CHECK_TYPE_1(object, _0)				CMRX_RPC_CHECK_TYPE(object, (_0))
#define CMRX_RPC_CHECK_TYPE_0(object, foo)				CMRX_RPC_CHECK_TYPE(object, ())

#define CMRX_RPC_CHECK_TYPE(object, args)				if (0) { object ## args }

/*
 * Checker to make sure that the layout of type being used to perform calls is correct.
 * The CMRX ABI requires, that the `vtable` member is the first member in the struct.
 * Otherwise kernel won't be able to locate the virtual method table during rpc_call()
 * execution.
 */

#define CMRX_RPC_INTERFACE_CHECKER_IMPL(service_instance) "Service " #service_instance " has not a valid layout! VTable does not come first!"
#define CMRX_RPC_INTERFACE_CHECKER_AUX(service_instance) CMRX_RPC_INTERFACE_CHECKER_IMPL(service_instance)
#define CMRX_RPC_INTERFACE_CHECKER(service_instance) _Static_assert(offsetof(typeof(*service_instance), vtable) == 0, CMRX_RPC_INTERFACE_CHECKER_AUX(service_instance))

/*
 * The master RPC call macro.
 * This macro will emit RPC syscall with reordered arguments.
 * Next types of arguments are checked against the called RPC method.
 * And last the ABI of the service is checked to be valid.
 */

#define CMRX_RPC_CALL(service_instance, method_name, ...)\
	RPC_EVALUATOR(RPC_GET_ARG_COUNT(__VA_ARGS__))(\
			(service_instance), \
			offsetof(typeof(*((service_instance)->vtable)), method_name) / sizeof(void *), \
			##__VA_ARGS__);\
	RPC_TYPE_CHECKER(RPC_GET_ARG_COUNT(__VA_ARGS__), (service_instance)->vtable->method_name, __VA_ARGS__) \
    CMRX_RPC_INTERFACE_CHECKER(service_instance)

/**
 * @ingroup api_rpc
 * @{
 */


/** User-visible way to perform remote procedure call.
 *
 * Calls service published by different thread and/or process. Service can take 0 to 4
 * arguments, which are passed down to it. Return value of service is passed back to the
 * caller.
 * Services are represented by structures allocated in service provider thread. These 
 * structures have to contain pointer to their virtual method table as their first member.
 * Virtual method table is then queried for presence of method name used to perform the call.
 * If such method exists, then arguments used to perform the call are checked against
 * method prototype.
 * @param service_instance address of service instance, which is being called
 * @param method_name name of method within service, which has to be called
 * @returns whatever value service returned
 */
#define rpc_call(service_instance, method_name, ...) CMRX_RPC_CALL(service_instance, method_name __VA_OPT__(,) __VA_ARGS__)

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
