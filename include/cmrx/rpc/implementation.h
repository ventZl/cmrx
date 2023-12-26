#include "internal/implementation.h"

/** @addtogroup api_rpc
 *
 * API to create remote procedure call services and to call them.
 *
 * @section api_rpc_overview RPC mechanism overview 
 * 
 * RPC mechanism is client-server-like mechanism based on interfaces. First, the interface
 * must exist. This interface defines a range of operations that can be performed on some 
 * object. Next, any process within CMRX is free to provide an implementation of this interface.
 * Then, it can define as many objects which support this implementation as it wants.
 *
 * Any other process (even the same) - client can then call the implemented operations on 
 * the object as long as it knows the address of the object in the memory. For the client, the
 * internal structure and content of the object are opaque and inaccessible. It can only call
 * methods this object support in the implementation. The validity of call is checked
 * at compile time. The validity of pointers is checked at runtime.
 *
 * @section api_rpc_interface RPC interfaces (vtables)
 *
 * RPC interfaces are defined in a form of type definition of structures containing pointers 
 * to functions. The sole purpose of interfaces is to declare a set of operations that are 
 * available for certain types of objects. Interfaces never provide any implementation, not 
 * even default one. Interfaces shall be as abstract as the topic of the interface possibly 
 * allows. The interface may consist of any amount of functions that have following limitations:
 *  * first argument to the function call must be pointer to the object 
 *  * other than the pointer to the object, functions can have 0 to 4 arguments that must be 
 *  * integers.
 * Interfaces never contain any data members, only pointer to functions. To make the code more
 * readable, there is a helper macro to identify the pointer to object, named @ref INSTANCE().
 * A type declaration, which consists exclusively of pointer-to-function members and these
 * members follow the rules for RPC API are called vtables.
 *
 * @section api_rpc_implementation Implementation of RPC interfaces
 *
 * Each process willing to provide RPC calls - the server - has to provide an implementation
 * of these interfaces. The implementation of interface is performed by defining functions
 * having prototypes equivalent to prototypes of vtable member functions. Unlike interfaces,
 * where the pointer-to-object argument has no specific type (as the interface is abstract),
 * with implementations it is necessary to type this pointer correctly. This is second 
 * responsibility of the @ref INSTANCE() macro. During the implementation phase, it typecasts
 * the `this` pointer to correct type, so strong type safety can be achieved.
 *
 * Any amount of processes within one firmware can implement one specific interface. There is 
 * no limitation on this. One process can even provide multiple implementations of the same
 * interface, for example in case that it manages multiple objects of different type, which 
 * all provide the same manipulation semantics.
 *
 * In order to publish an implementation of some interface, a process must create a variable
 * using desired interface's vtable structure. This variable will be initialized to contain
 * pointers to functions that implement the interface. This variable **must** be defined within
 * one of process' modules and **must** be prefixed by the @ref VTABLE keyword otherwise the
 * kernel will refuse calls using such implementation for security reasons. This variable is 
 * known as vtable instance.
 *
 * @section api_rpc_objects RPC objects (services)
 *
 * After the interface has been implemented for certain process, it is possible to create 
 * objects which can be manipulated using the given interface. Objects serve as the actual
 * data storage for the implementation of the interface. Object structure is thus bound with
 * the interface implementation. The only limitation given to objects is that they have to 
 * contain a pointer to vtable instance, which is the first member of the XXXXXXXXXX.
 * Remote procedure call mechanism in CMRX is built on top of class-like interfaces.
 * Party, which provides an interface, always provides two entities to the caller. First of them
 * is opaque pointer to the interface instance (let's call it object). Object resides in memory
 * space of it's provider and it is usually inaccesible by the user, because memory areas of
 * different processes are separated by the means of MPU. Second entity provided to the caller
 * is interface description (let's call it virtual method table). This table contains names
 * and prototypes of methods, which can be called for any given object. Caller can then call
 * any method using wrapper rpc_call, method name and object pointer.
 *
 * @section api_rpc_instances RPC service instances
 *
 * wadda wadda
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
