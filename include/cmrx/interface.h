#pragma once

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

/** Safety belt for incomplete interface definitions
 *
 * Typedef catching situations, when interface implementation is performed, 
 * but if wasn't declared, to which interface methods actually belong.
 *
 * Normally, it is expected that there is 
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~
 * #define CLASS struct <something>
 * ~~~~~~~~~~~~~~~~~~~~~~~
 *
 * in front of interface definition block, which tells compiler, how to typecast
 * references to class instance. Existence of this typedef causes that error is
 * a bit less misleading than it would be without this typedef.
 */
typedef void * CLASS;

/** Macro used to refer to interface in prototypes and virtual method tables.
 *
 * This macro is used in place of method argument to denote interface instance pointer.
 * It always must be in place of first method argument, otherwise calls to method will
 * misbehave and most probably also crash thread. It is typed as `void *` to enable
 * inheritance and virtual methods.
 *
 * @note We know that this is ugly.
 */
#define SELF void * this

/** Macro used to refer to interface in bodies of virtual methods.
 *
 * This macro is used whenever body of method needs to refer to the instance of
 * interface, it was called with. Note, that prototypes are always using UPCASE version,
 * while body always uses lowercase version of `self`. This macro expands to typecast to
 * type declared previously by the macro CLASS. This means, that every block of virtual
 * method implementation **must** be preceded by declaration of macro CLASS, which holds
 * **full** type name of interface, for which implementation is going to be provided.
 */
#define self ((CLASS *) this)

/** @} */
