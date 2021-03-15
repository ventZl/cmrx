#pragma once

#include <stdint.h>

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

