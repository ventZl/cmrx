Cortex-M Realtime eXecutive
===========================

This is realtime, mikrokernel operating system targeted towards Cortex-M
microcontrollers having udable support for MPU. Currently microcontrollers based
on Cortex-M0+, Cortex-M3, Cortex-M4 are known to be supported.

This operating system provides only several basic services needed to make things
happen:

* memory protection and isolation of processes
* multi-threaded scheduler
* support for fast userspace mutexes (TBD)

Planned features:

* extension of basic IPC: remote procedure calls, shared memory

In line with mikrokernel architecture, operating system itself does not offer
any non-essential services. Instead of this, there is a mechanism, which allows
implementation of various services and system calls using standalone processes,
which are isolated from the rest of system. There will be a quick way, how to
call these services once.
