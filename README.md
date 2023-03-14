C Microkernel Realtime eXecutive
================================

This is realtime, mikrokernel operating system with fully static operation currently 
targeted towards Cortex-M microcontrollers having usable support for MPU. Currently 
microcontrollers based on Cortex-M0+, Cortex-M3, Cortex-M4 and Cortex-M7 are known 
to be supported.

This operating system provides only several basic services needed to make things
happen:

* memory protection and isolation of processes
* multi-threaded, multi-process scheduler

Additionally CMRX offers basic set of IPC features:

* support for fast userspace mutexes
* thread management: on demand creating, disposing and joining of other threads
* signal delivery: threads can register signal handlers and other threads can send 
  them signals
* remote procedure call: kernel can dispatch call performed using interface and 
  method ID and then provide result back to the caller.

TBD:
* registering RPC services as syscalls

In line with microkernel architecture, operating system itself does not offer
any non-essential services. Instead of this, there is a mechanism, which allows
implementation of various services and system calls using standalone processes,
which are isolated from the rest of system.

CMRX does not offer any specific set of higher RTOS API. The only API (optional) 
CMRX offers is POSIX-compatible threading and signal API. Absence of any
specific RTOS API is intentional. No defined higher API means, that on top of
CMRX kernel, any API can be implemented, be it CMSIS-RTOS or OSEK OS.
