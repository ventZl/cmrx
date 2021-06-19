\page man Introduction

Introduction into CMRX realtime operating system
================================================

CMRX is a realtime operating system targeting Cortex-M hardware built around several core ideas:

 * isolate memory space of different processes to improve robustness. If any application experiences
   a bug and accesses memory, it shouldn't access, this is detected and application is terminated.
 * use minimalistic kernel, which only provides basic functionality for process and thread management 
   and interprocess communication. Any other services which may be of general use, should be 
   implemented as servers available via hosted syscalls.
 * application software should never run before kernel is fully initialized. No pre-scheduler 
   initialization other than generic GPIO / clock setup. everything then happens inside application
   and/or interrupt service routines.
