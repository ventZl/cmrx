\page intro Introduction

CMRX is a realtime operating system targeting Cortex-M hardware built around several core ideas:

 * isolate memory space of different processes to improve robustness. If any application experiences
   a bug and accesses memory, it shouldn't access, this is detected and application is terminated.
 * use minimalistic kernel, which only provides basic functionality for process and thread management
   and interprocess communication. Any other services which may be of general use, should be 
   implemented as servers available via hosted syscalls.
 * application software should never run before kernel is fully initialized. No pre-scheduler 
   initialization other than generic GPIO / clock setup. Everything then happens inside application
   and/or interrupt service routines
 * operation should be fully static. The operating system can be statically configured to provide
   predetermined amount of resources of each and every kind. Application can be written in a way
   that absolutely no dynamic allocation happens on runtime. All delays are deterministic.

The general idea is to provide full memory protection while enabling fully static operation on 
microcontroller-class hardware. While being a micro-kernel, CMRX still offers wide variety of
features and services:

  * multi-threaded environment with POSIX-compatible thread management API 
  * per-process sandboxing of memory accesses
  * full memory protection of all the application code including stack overflows and invalid
    reads/writes
  * prioritized, preemptive, or purely cooperative scheduler
  * cross-process remote procedure calling mechanism supporting memory sharing
  * signals and timers
  * interrupt services for ISR interaction with kernel objects
  * userspace mutexes

Being a micro-kernel, there is no goal in adding each and every possible feature. Rather the aim is
at providing kernel infrastructure, which will enable implementation of pretty much any desired
feature in an effective way, while retaining static operation and being fully memory protected. Due
to this, there are RTOS features, you won't find in CMRX list of features:
  
  * message queues
  * cross-process mutexes
  * semaphores
  * stream buffers
  * heap memory allocator
  * events
  * unconditionally shared memory

Any of these features may be developed as a server, which offers services via hosted syscall. This
way, basically any API can be supported without being forced to carry the implementation of any
other and generally unnecessary API chosen by us.

We also don't feel need to be a dog and a cat at the same time, so the application software is only
considered to be running as non-privileged, memory protection-constrained userspace threads. There is
no support for thread-less operation. While memory protection can technically be turned off, this
mode of operation is also not supported. As of now there is no support for running kernel-level
(privileged) threads. The aim is to run additional services as userspace servers while providing
access via system call registration (so called hosted syscalls).
