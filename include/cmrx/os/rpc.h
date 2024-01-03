/** @defgroup os_rpc Remote Procedure Calls
 *
 * @ingroup os
 *
 * This kernel implementation of RPC mechanism.
 * @{
 */
#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <cmrx/defines.h>

/** Constant denoting that the VTable pointer is not valid */
#define E_VTABLE_UNKNOWN			0xFF

struct RPC_Service_t_;

typedef struct RPC_Service_t_ RPC_Service_t;

/** Calling signature of a RPC call.
 * Any function that wants to implement a RPC call has to have this signature.
 * If function won't have use for any of `arg0`, `arg1`, `arg2`, `arg3`,
 * they can be omitted right-to-left. The argument `service` cannot be omitted and its position
 * as the first argument has to be maintained.
 * RPC calls are limited to use the general purpose registers only. Floating point registers
 * cannot be used as it might not be possible to determine which registers were actually used
 * to pass the arguments. This restricts the use of floating-point types to perform RPC calls.
 * @param service Address of RPC object which was used to perform this RPC call. Works like
 * self` variable in Python.
 * @param arg0 optional argument to RPC call. Can only be of integral 32-bit large type
 * @param arg1 optional argument to RPC call. Can only be of integral 32-bit large type
 * @param arg2 optional argument to RPC call. Can only be of integral 32-bit large type
 * @param arg3 optional argument to RPC call. Can only be of integral 32-bit large type
 */
typedef int (*RPC_Method_t)(RPC_Service_t * service, unsigned arg0, unsigned arg1, unsigned arg2, unsigned arg3);

/** Type definition of VTable.
 * VTable is technically just an array of pointers to functions.
 * To make things more user friendly, in real world cases, VTables are 
 * usually a structures whose members are pointers to functions. The memory
 * layout of both cases is the same but structures allows for named members.
 */
typedef RPC_Method_t * VTable_t ;

/** Basic structure of any RPC object.
 * The RPC API requires, that the first member of any RPC callable service
 * is the pointer to the VTable. This is checked at compile time and build
 * will fail if this requirement is not satisfied.
 * Real world RPC object won't contain just the pointer to the VTable but 
 * members containin the state of the RPC object will follow.
 */
struct RPC_Service_t_ {
    /** Address of the VTable which contains pointers to functions that
     * implement this service.
     */
	VTable_t * vtable;
};

/** Identify process which owns the VTable.
 * This function will find the process which defined this vtable.
 * @param vtable address of the vtable retrieved from the RPC object.
 * @returns process ID of the owning process or @ref E_VTABLE_UNKNOWN 
 * if vtable address does not belong to any known process.
 */
Process_t get_vtable_process(VTable_t * vtable);

/** Add new process ID to the stack of RPC calls.
 * Registers new process ID in thread's stack of RPC call owning processes.
 * This stack records which process is "owning" this thread while RPC call
 * is in effect. While RPC call can technically perform another RPC call,
 * this is a stack rather than plain field. New member is always added on
 * top of the stack unless stack is full.
 * @param process_id ID of the process owning the RPC method
 * @returns true if process was added, false if stack is already full
 */
bool rpc_stack_push(Process_t process_id);

/** Remove the last entry in the RPC stack.
 * Removes the most recently added entry from thread's stack of RPC call
 * owning processes. Will do nothing if stack is already empty.
 * @returns new depth of the stack. Returns 0 if stack is empty.
 */
int rpc_stack_pop();

/** Retrieve the topmost process ID in thread's RPC call stack.
 * Returns the process ID of the most recently added entry in thread's 
 * stack of RPC call owning processes.
 * @returns Most recently added process_ID or @ref E_VTABLE_UNKNOWN if 
 * RPC call stack is empty.
 */
Process_t rpc_stack_top();

/** Kernel implementation of rpc_call syscall.
 *
 * This routine performs remote procedure call. It digs for 
 * 5th and 6th argument passed to @ref _rpc_call() on thread stack. Retrieves
 * address of called method from service VMT and synthesizes stack frame for
 * jumping into this method. Arguments used to call _rpc_call() are passed to
 * callee.
 */
int os_rpc_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/** Kernel implementation of rpc_return syscall.
 *
 * This routine unwinds stack frame used to call RPC method and passes return
 * value from RPC to the caller.
 */
int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/** @} */
