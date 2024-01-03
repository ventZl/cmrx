/** @addtogroup arch_arch
 *
 * @{ 
 */

/** Kernel implementation of rpc_call syscall.
 * This syscall has to validate the RPC service and method IDs, determine the address
 * of RPC method and owning process. Then it has to transfer the control to RPC method
 * in a manner that:
 * * method called will be able to access the first four arguments given to the @ref rpc_call() call.
 * * when method returns, the @ref os_rpc_return() is triggered and will transfer the control back
 * * to the calling code and process.
 */
int os_rpc_call(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/** Kernel implementation of rpc_return syscall.
 * This syscall has to return the control back to the code which called @ref rpc_call. This
 * has to be done in a way that the calling code will be able to access the return value of
 * the RPC method.
 */
int os_rpc_return(uint32_t arg0, uint32_t arg1, uint32_t arg2, uint32_t arg3);

/// @}
