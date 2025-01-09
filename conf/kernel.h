/** @defgroup os_config Kernel runtime configuration
 * Compile-time configuration of kernel runtime parameters.
 * Do not modify this file directly. Use CMake options in your top-level CMakeLists.txt
 * to alter generation of this file. Any changes done to this file may be overwritten
 * by the build system!
 * @{
 */
#pragma once

/** This turns on memory protection globally.
 * This is a partially obsolete and unsupported option. CMRX does not support running 
 * without memory protection activated as certain mechanism in the kernel are directly
 * expecting MPU will intervene on specific bad behavior of the application. */
#define KERNEL_HAS_MEMORY_PROTECTION

/** How many MPU regions are saved per thread */
#define MPU_STATE_SIZE			7

/** How many MPU regions are always used based on in which process thread is hosted */
#define MPU_HOSTED_STATE_SIZE	4

/** How many MPU regions can process define */
#define OS_TASK_MPU_REGIONS		5

/** How big stack is? In bytes */
#define OS_STACK_SIZE  @OS_STACK_SIZE@
#ifndef OS_STACK_SIZE
#   error "Stack size not defined! Fix your CMakeLists.txt!"
#endif

/** How many threads can exist */
#define OS_THREADS				@OS_THREADS@

/** How many stacks can be allocated */
#define OS_STACKS				@OS_STACKS@

/** How many processes can be allocated */
#define OS_PROCESSES 			@OS_PROCESSES@

/** How many sleeping threads can exist */
#define SLEEPERS_MAX			(2 * OS_THREADS)

#cmakedefine CMRX_ARCH_SMP_SUPPORTED
#ifdef CMRX_ARCH_SMP_SUPPORTED
#    define OS_NUM_CORES    @CMRX_OS_NUM_CORES@
#endif

#define OS_NOTIFICATION_BUFFER_SIZE     16

/** Enable / disable kernel tracing */
#cmakedefine CMRX_KERNEL_TRACING

/** @} */
