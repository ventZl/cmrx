Cortex-M Realtime eXecutive
===========================

This is realtime, mikrokernel operating system targeted towards Cortex-M
microcontrollers having udable support for MPU. Currently microcontrollers based
on Cortex-M0+, Cortex-M3, Cortex-M4 are known to be supported.

This operating system provides only several basic services needed to make things
happen:

* memory protection and isolation of processes
* multi-threaded scheduler

Additionally CMRX offers basic set of IPC features:

* support for fast userspace mutexes
* thread management: on demand creating, disposing and joining of other threads
* signal delivery: threads can register signal handlers and other threads can send 
  them signals
* remote procedure call: kernel can dispatch call performed using interface and 
  method ID and then provide result back to the caller.

TBD:
* shared memory support
* registering of RPC services as syscalls

In line with mikrokernel architecture, operating system itself does not offer
any non-essential services. Instead of this, there is a mechanism, which allows
implementation of various services and system calls using standalone processes,
which are isolated from the rest of system. There will be a quick way, how to
call these services once.
