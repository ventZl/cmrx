#pragma once

/** @defgroup arch Architecture support
 * @ingroup os 
 * Layer containing bits that are specific to each supported architecture.
 *
 * @details
 * Most of the kernel is written in platform-independent style. Still this platform-indepentent
 * kernel has to have architecture support layer that does the low level work. Here is the 
 * documentation of these parts. 
 *
 * Section @ref arch_arch documents the API you have to provide if you want to port CMRX to
 * a new architecture. This usually translates to new **CPU** architecture as different platforms
 * using some already supported CPU don't need kernel to be ported again.
 *
 * Other sections describe port(s) already done.
 *
 * Architecture layer linked to the kernel depends on setting of CMake variable @ref CMRX_ARCH.
 * The architecture layer updates the `os` target to provide portions which are necessary to 
 * support desired architecture.
 *
 */

/** @defgroup arch_arch Abstract machine
 * @ingroup arch
 * Description of CMRX abstract machine. CMRX expects to run on abstract machine providing
 * functionality that can be accessed by API described here. CMRX doesn't care nor access the
 * underlying platform by any other means and makes no strong assumptions on the target
 * platform other than assumptions presented here. The target plaform described by the abstract
 * machine can be whatever that meets requirements stated here.
 *
 * This list is by no means definitive. Each architecture will require its own mechanisms to
 * be implemented so the kernel can actually work. Mechanisms CMRX requires to be present in
 * the architecture and be implemented are:
 *  * service / system call mechanism that allows to call the kernel from usermode code
 *  * memory protection mechanism 
 *  * mechanism allowing to schedule and then perform thread switching
 *  * mechanisms to define and collect system call lists and lists of processes/tasks
 *
 * Similarly, some of kernel syscalls are directly implemented by the architecture support 
 * layer as there is no way for kernel to know how platform will implement given mechanisms.
 * This covers mostly the RPC call / return syscalls and signal delivery.

To create a port of CMRX to the new architecture a few steps are needed. This guide
roughly describes them in general terms and outlines items that have to be provided which
are not immediately obvious from the portin layer API.

In the following text, several terms will be used:

* Architecture - refers to the CPU family which determines most of target CPU
  functionality. In case of CMRX, ports are mainly done to support certain CPU
  architecture rather than specific platform (see next). Examples of architectures are:
  ARM (Cortex-M), RISC-V (RV32/64-E).

* Platform - refers to specific subfamily of CPU. This subfamily may further determine
  presence of absence of certain CPU features. CMRX mostly don't care about presence of
  features it does not directly support or require. Features, that are required by CMRX
  (especially memory protection) must be present, otherwise it is not possible to port
  CMRX to such architecture reliably. In case of ARM, such platforms might be Cortex-M0+,
  Cortex-M4 or Cortex-M4F.

* Port - refers to specific vendor implementation of the platform. In case of CMRX,
  specifically for the ARM architecture, all ports are covered by generic port named
  "CMSIS". This ports expects that your vendor's SDK provides CMSIS-compatible headers.
  The CMRX build system supports creation of ports, but as long as there is technical
  solution available that does not require port creation, it should be avoided. In CMRX
  terminology port refers to the same thing as term "HAL" does.

@section anatomy Anatomy of CMRX abstract machine

CMRX kernel is divided into two parts which are interconnected and together form a full CMRX
kernel. One part is the platform-independent code that mostly covers the CMRX API and
common functionality, like scheduler, timer implementation, etc. Another part provides
architecture- and platform-specific functionality needed for the former part to be able to
execute.

Following text describes the latter, the abtract machine. Abstract machine API basically consists
of three (possibly four) parts:

* CMake script to define actions specific to correctly support the target architecture
* C headers containing definitions both required by platform-independent part and required
  internally by the port itself.
* C sources containing implementation of the abstract machine on given platform.
* Optionally, there might be some scripts required to support architecture-specific
  features.

@section script CMake script

Architecture support script is expected to be stored in
`<root_dir>/cmake/arch/<architecture>/<port>/CMRX.cmake` file. This file will be included
automatically, once `include(CMRX)` is hit in project's `CMakeLists.txt`.

This file is expected to define two functions:
* add_firmware()
* target_add_application()

@section headers C headers

All headers that provide support for some specific platform shall be stored in
`<root_dir>/include/cmrx/<architecture>/<port>/arch/` directory. This directory will
automatically be added into include paths of CMRX kernel. The code can then refer to
headers inside this directory as using `#include <arch/header.h>`. 

This directory shall contain all the header files your port contains. There are no limits
on what files you create and how you name them. CMRX platform-independent part will not
include almost none of them.

The platform-independent part of CMRX expects just a few files to exist in this directory.
They will be directly included by the platform-independent part so they shall only contain
entities described below and shall not include any other headers, if possible.

@subsection context_h context.h

This file contains API to request context switch to happen. This has to be implemented as
a function.

* @ref os_request_context_switch() - initiates or cancells context switch. In certain situations
  kernel may decide that it wants to cancel a pending request to perform context switch. Request
  shall not be performed immediately, yet after the current system call or interrupt handler is
  done.

@subsection corelocal_h corelocal.h

This file has to contain two entities. They might be implemented as macros, static inline
functions or any other kind of function, as needed by the platform.

* @ref coreid() - provides ID of the currently running core. For uni-processor systems, this
  can be a macro hardcoded to return value of 0.
* @ref os_smp_lock() / @ref os_smp_unlock() - provides SMP-aware locking mechanism to instantiate
  system-wide critical section.
* @ref os_core_lock() / @ref os_core_unlock() - provides core-local locking mechanism to instantiate
  local critical section.
* @ref os_core_sleep() - halt the current core in order to save power. Core must be restartable
  by the means of interrupt.
* OS_NUM_CORES - usually a macro, that provides information on amount of cores present in 
  the system.

@subsection mpu_h mpu.h

This file has to contain one type definition. Portable part of kernel expects its
existence:

* struct MPU_Registers - the internal structure of this type is not important for the
  portable part of CMRX kernel, but the size of this structure must be large enough so
  that the porting layer for the architecture will be able to store memory protection unit
  state of CPU when swapping threads in and out. The design of this structure is entirely
  up on the designed of the port. Portable part of CMRX doesn't use the data stored there.

* @ref os_memory_protection_start() - configure the underlying hardware to enforce memory proctecion.
* @ref mpu_init_stack() - adjust internal MPU-related structures to support the stack of given thread.
* @ref mpu_restore() - restore MPU hardware into state suitable for given thread to run.
* @ref os_memory_protection_stop() - configure the underlying hardware to not enforce memory protection.
  This is used just before the kernel reboots to disable  memory protection in user mode.

@subsection rpc_h rpc.h

This file implements two of the syscalls which are related to the RPC mechanism: Entering and
leaving the RPC calls.

* @ref os_rpc_call() - perform RPC call
* @ref os_rpc_return() - return from RPC call back to the caller context

@subsection runtime_h runtime.h

This file implements some architecture-specific callbacks and structures. It allows the architecture
to customize the thread control block and react to certain events.

* struct @ref Arch_State_t - this structure allows the port to put additional architecture-specific data into TCB.
* @ref os_init_arch() - called when operating system boots. Once per boot.
* @ref os_init_core() - called when the core is initialized during boot. Once per each core that is
  being booted by the OS.

@subsection sched_h sched.h

This file implements primitives that are used to perform certain architecture specific tasks related
to scheduling.

* @ref os_thread_initialize_arch() - perform architecture-specific way of initializing the freshly-created thread
* @ref os_process_create() - perform architecture-specific way of initializing the freshly-created process
* @ref os_boot_thread() - boot the kernel userspace by starting executing the given thread in the
  user mode of the CPU being protected by the MPU.
* @ref os_kernel_shutdown() - return to the privileged user mode execution similar to one before the
  kernel was started up.
* @ref os_reset_cpu() - perform CPU reset. This should make sure the software starts executing again in
  known "clean" state of the CPU.
* @ref os_set_syscall_return_value() - set system call return value

@subsection sysenter_h sysenter.h 

This file has to define two objects:

* __SYSCALL - this macro shall expand to list of attributes that function serving as the
  syscall entrypoint has to have. It can be empty if there are none, but has to be
  defined.

* __SVC() - this can be either macro or a function, depending on the architecture. It
  shall expand to or directly execute architecture-specific means of calling the system
  calls. It has to accept one argument, which is a system call ID.

@section sources Port implementation sources

Port has to implement certain functions that are expected to be provided by it. Sources of
the port can be stored in directory `<root_dir>/src/os/arch/<architecture>`. CMRX build
system will expect that `CMakeLists.txt` file exist there and will include it automatically.
This `CMakeLists.txt` file shall update the target `os` to include sources located in the
architecture support directory. This way the platform-independent portion will be extended
by the necessary platform support. If platform support requires linking of any additional
libraries, such as HAL, then commands to let the `os` target link them should be present
in CMakeLists.txt in this directory as well. 

These sources shall provide implementation of functions outlined in this 
section of the manual. If port fails to provide the implementation for any of them, build
will most probably fail. Port has to accept the API and semantics of functions.

To provide full set of functionality, some architecture-specific mechanisms might need to
be implemented by the port. CMRX porting layer does not directly prescribe, which these are as it is not possible to determine them in advance. It is up to the designer of the port to figure out what exact mechanisms are these.

For CMRX purposes, mechanisms, which are known to be needed are:

* mechanism to handle "service call". The "service call" is a mechanism of transferring
  control from the user space into kernel space. This mechanism has to be able to provide
  numeric value of service which was called.

Optionally, additional mechanisms can be implemented in the port, such as:

* mechanism to handle certain types of program faults. This handler might be useful in
  determining if the root cause of the fault was a memory protection violation and to send
  the signal to kill the failing thread.

* mechanism to defer thread switch. CMRX is written in a way, that thread switch is
  expected to be executed after the system call / interrupt service handler or kernel
  callback has finished its run. If target architecture doesn't support this behavior,
  this mechanism does not need to be implemented.

 * @{
 */

/** Provides count of statically initialized threads.
 * @returns amount of threads that have to be statically initialized */
unsigned static_init_thread_count();

/** Provides address of statically initialized thread table.
 * @returns address of table containing details of statically initialized threads */
const struct OS_thread_create_t * static_init_thread_table();

/** Provides count of statically initialized processes
 * @returns amount of processes that have to be statically initialized */
unsigned static_init_process_count();

/** Provides address of statically intialized process table.
 * @returns address of table containins details of statically initialized processes */
const struct OS_process_definition_t * static_init_process_table();

/** @} */
