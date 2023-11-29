#include "internal/implementation.h"

/** @addtogroup api_rpc
 *
 * Remote procedure call mechanism in CMRX is built on top of class-like interfaces.
 * Party, which provides an interface, always provides two entities to the caller. First of them
 * is opaque pointer to the interface instance (let's call it object). Object resides in memory
 * space of it's provider and it is usually inaccesible by the user, because memory areas of
 * different processes are separated by the means of MPU. Second entity provided to the caller
 * is interface description (let's call it virtual method table). This table contains names
 * and prototypes of methods, which can be called for any given object. Caller can then call
 * any method using wrapper rpc_call, method name and object pointer.
 *
 * Caller thus does not need any details on what is actual organization of objects internals,
 * neither does need to care about calling the right method to perform correct operation on
 * correct object type. On the other side, this organization offers some degree of abstraction,
 * because offering party can declare, that interface is of some generic kind (such as ComSource),
 * but internally, it can implement completely distinct object, which only offers interface
 * compatible with one announced.
 *
 * Another significant property of remote procedure calls is that they are truly remote.
 * Caller and callee may reside in two completely different processes. RPC call is performed via
 * kernel, which will ensure, that thread, which called RPC method, will gain access to address
 * space of callee process exclusively just for the duration of method call. Immediately after 
 * method finishes, access to address space of callee is removed. On the other hand, callee won't
 * gain automatic access to caller's address space. This arrangement minimizes possible damage, 
 * which can be caused either by malfunctioning caller, or callee. In order to enable sharing 
 * of larger portions of memory between caller and callee, there is separate mechanism of
 * @ref api_shared, which enables callee access to specified portions of caller's address space.
 *
 *
 *
 * @{
 */

// we might have included rpc/interface.h before
#undef INSTANCE

/** Begin implementation of interface for service.
 * Use of this macro starts implementation of methods for 
 * certain service. It enables use of the `this` term to 
 * refer to the service instance current during the method 
 * call.
 * @param service name of type which describes the service whose methods are being implemented
 */

#define IMPLEMENTATION(service) CMRX_IMPLEMENTATION_HELPER(service, CMRX__INTERFACE__COUNTER)

/** Extended version of IMPLEMENTATION macro.
 * Use of this macro starts implementation of methods for 
 * certain service. It enables use of the `this` term to 
 * refer to the service instance current during the method 
 * call. Another function of this macro is that it generates 
 * a static assert to check, if the type of interface 
 * implemented by the service matches the one, which is expected
 * here
 * @param service type of structure, which holds instances of service
 * @param interface type describing virtual method table of the implemented interface
 */
#define IMPLEMENTATION_OF(service, interface) CMRX_IMPLEMENTATION_HELPER(service, CMRX__INTERFACE__COUNTER);\
_Static_assert(CMRX_CHECK_INTERFACE_MATCH(service, interface), CMRX_IMPLEMENTATION_TYPE_HELPER(service, interface))

/** Mark function argument as reference to current service instance.
 * Syntactic sugar to make argument type-compatible with interface declaration.
 * This allows to work with any specific type of service inside its implementation
 * and use of such specialized implementations to initialize generic interface.
 * @note As a matter of fact, the only allowed argument of this macro is `this`
 * 
 * @note Due to the API of RPC calls, you *HAVE* to use this macro on 1st argument
 * of method definition.
 */
#define INSTANCE(a) void * a ## _

/** Access current service instance.
 * `this` provides access to the current service instance.
 * It is always typed correctly to access members of service currently being 
 * implemented.
 */
#define this CMRX_THIS_HELPER(CMRX__INTERFACE__COUNTER)

/** @} */
