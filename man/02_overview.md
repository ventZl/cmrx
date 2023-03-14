@page overview Overview

This page provides and overview of basic concepts in CMRX environment. You shall
make yourself familiar with all the concepts listed here in order to understand
how CMRX environment behaves and how to achieve certain goals.

@subpage concepts Basic concepts

@subpage mem_model Memory model

@subpage rpc_intro Remote Procedure Call intro

@subpage dev_env Build environment

@page concepts Basic concepts
If using CMRX, there are some basic concepts enforced by the kernel itself.

Application code is organized into processes.

Process is a collection of accessible 
memory, threads and RPC services. Any userspace code has either run as ISR handler, or
be launched from userspace thread, which is created in context of userspace process.
There is no way to run kernel-privileged code outside interrupts. Currently, processes
have to be statically declared at compile time and it is not possible to create new 
processes during runtime.

Thread is an execution context within process. Thread has its private stack, priority,
and hosting process. Threads can be auto-started on system startup after processes
are created. This effectivelly allows fully-static operation. Or threads can be created 
during runtime. It is not possible to create a thread inside foreign process.

Memory, which is accessible for the userspace code can generally be divided into four
types:

  * code and read-only data - this memory is generally available for read and execute to
    anyone. Any thread running within any process can read and execute any part of FLASH
  * read-write process data - this memory is composed of .bss and .data sections belonging
    to certain process. This memory is available for reading and writing to all threads
    being hosted in the same process.
  * read-write shared data - this memory is composed of variables tagged as belonging into
    .shared section. This memory is available to all threads which are originating in the
    same process.
  * stack - stack is thread-private. No other thread, even from within the same process,
    can access stack of any other thread.

In general, writable memory is not executable. An attempt to execute from RAM will end up
with thread termination.

It is possible to launch multiple instances of same code, all of them having different
data. This is due to the fact that code, even process-specific, is visible to all the 
userspace.

RPC services are the only supported means of inter-process communication. While threads
belonging to the same process all share the same memory, there is no means of sharing memory
belonging to any other process unconditionally. One has to use RPC to execute code within 
context of other process.

Any process can create one or more services. These services are composed of methods the service
offers. It is then possible to create as many instances of these services as needed. Service
declaration is exclusively static operation done during compile time. Instance creation is possible
during the compile time, enabling fully static creation of RPC services, or optionally during
runtime.

Instances can then be used to call methods of services in cross-process fashion. RPC call
dispatch is provided by the kernel in fully memory protected fashion. While performing RPC call,
certain, statically defined regions of memory can be shared between the caller and callee. Only code
the service owner specified, can be executed via RPC call.


@page mem_model Memory Model
Processes running in CMRX environment have memory protection use enforced.
Therefore the application (or, better: process) can only access resources, which
belong to it. In CMRX, the process is the governing entity, when it comes to what
resources can be accessed from within specific execution context.

Code and read-only data
-----------------------

All code, which doesn't belong to kernel itself (with some very specific
exceptions) has to belong to one of processes declared at build time.
Process has to have name and is given access to certain memory areas.

Due to limitations of Cortex-M hardware, whole flash region is marked as read +
execute. It would be extremely difficult to provide working implementation of
memory protection which would allow use of library functions if code was under
memory protection. And it would probably impossible to do that statically. This
design decision has effect you have to be aware of as a developer: every code
can read everything in flash, including any possible secrets stored there.

When it comes to RAM, the situation is completely different. Any process can
only access RAM claimed by variables being defined by process' code.
Process RAM region is generally divided into three separate sections:

Static variables region
-----------------------

This region is composed of all variables defined in all compilation units
belonging to some specific process. It is sufficient to define a variable in
compilation unit, which ends up being compiled as a result of add_application()
call and that variable will be part of process' memory space. Such memory
can only be accessed by threads running in the context of owning process. This
means, that all the code of all compilation units of given process can
always access all such memory.

You don't need to treat those variables in any special way in order for your
code to work properly. If variables are non-static (i.e. globally visible), you
can access them from every compilation unit belonging to owning process.

Shared variables region
-----------------------

This region is composed of variables defined in compilation units belonging to
some specific process, which has their definition prefixed by keyword
`SHARED`. Doing so will relocate given variable into shared region. Under normal
conditions, access rules to shared variables is the same as for static
variables. All the code of all compilation units belonging to the owning
process can access all the shared variables.

The situation changes, when a \ref rpc_intro is performed. While remote
procedure call is in flight, called procedure has access to shared memory of
calling process. This is effectively the only way of passing larger amount of 
data between processes.

Stack
-----

Stack is automatically allocated for each and every thread running in system.
Stack is thread-private and even no two threads of same process can access each
other's stack. Stack is automatically reclaimed by the kernel, once process
ends.

Memory-mapped IO
----------------

There is one region, which is custom-defined by the process designer.
Common use case of this region happens, when the process is a
device driver of some kind and needs to access the hardware directly. As device drivers
are just userspace processess in microkernel world, any such process needs to gain
access to memory region which allows control of given peripheral.

\page rpc_intro Remote Procedure Calls

Remote procedure calls are one of few existing ways of communicating between
processes. While threads can use mutexes (as all threads of same process have
access to same memory and therefore to same mutexes), threads of different
processes can't all access the same memory regions all the time.

Remote procedure calling has, as pretty much any IPC mechanism two ends. The
callee end and the caller end. Callee end is the end which ends up being called
as a result of RPC and we will call it "server" in further text. Caller is the
one, who will perform the call, once it decides it is right time to do so. We
will call it "client" for short.

High level design of RPC mechanism
----------------------------------

From high level perspective, RPC interface works in a way similar to C++
objects. Server process can declare one or more services, which are composed 
of methods. Methods are callable via the instance of the service.

From the server perspective, the service instance is a fully defined structure,
which holds both data and has assigned a set of callable service methods.

From the caller perspective, the service instance is an opaque pointer. This
pointer, once obtained, can be used to call service methods. Internally, the
object pointed to contains link to service method table. If caller uses this
pointer to call service method, this call gets resolved on compile time. If
method with requested name does not exist in target service, the compilation
fails.

Despite the fact, that the called method is resolved during the compile time,
the call itself is performed based on the service status during the runtime.
Compile time resolution makes sure that target method actually exists within
referenced service and composes kernel call to perform RPC call.

Later, during the runtime, kernel resolves service method address and passes
control to the provided address. At no time, the calling process has access to
service object, despite it's ability to call service methods. Runtime resolution
is done by the kernel.

RPC service
-------------

RPC service is a structure, which lists pointer-to-function kind of members.
This is a typical way of writing "C classes". There are no special requirements
for internal organization of this structure except of typical best practices in
writing reusable interfaces.

There are limitations given by RPC mechanism in CMRX though. Any method of RPC
interface has to have 1 up to 5 arguments. First arguments **must** be pointer
to instance. This semantics is same as used in Python methods. To simplify
things, you can use all caps `SELF` to fulfill that need. Remaining four
arguments can be of any scalar type. Method can return any scalar type, but does
not need to return anything.

Method entry in class may look like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void (*method_name)(INSTANCE(this), int arg);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

RPC object
----------

RPC object holds the data, RPC service provides access to. It is plain C
structure with only one requirement. First member of this struct **must** be a
pointer to the RPC service, which provides access to this object and this member
**must** be called `vtable`. Amount, types, structure and order of remaining
items is completely up to application developer. Even removal or reordering of
items within structure won't compromise binary compatibility, as long as your
service can cope with it.

Becoming a RPC server
---------------------

In order for some application to define a RPC server, it has to do quite a few
things:

Declare interfaces
------------------

Very first step is to define the interface. Interface is a structure visible
both to the server and the client, which defines what actions any given service
supports. Interface is purely procedural and does not support publishing of data
members. Interface has a form of structure containing pointer-to-function
members.

Declare services
------------------

After interfaces were defined (or adopted), the structure of the service itself
has to be provided. This is typically a structure, which holds all the state of
the service. This structure must contain a link to the interface, this service
implements.

In CMRX, the internals of the structure are never accessible outside
the owning process. So in practice, it is possible to provide two declarations
of the service: One, fully specified with all the data members. This one is used
internally while writing the service implementation. Another, containing only
the reference to interface implemented may be provided for callers.

Above is not a must, even if caller knows the internal structure of the
interface, it is not possible to access it. Not even in read-only manner.

Define service implementation
-----------------------------

Next, it is necessary to provide implementation of all functions declared in
interface the service implements. Once these functions are implemented, they can
be put together into interface implementation table. This is an instance of
interface, where function pointers point to actual functions just created.

If any interface provides some function, then it is mandatory to implement it.
No pointer in interface can point to NULL. Doing so will crash any caller which
attempts to call such interface method.

Instances of interfaces have to be marked by special keyword `VTABLE`.
This keyword serves the purpose of putting such variable into
VTABLE "region" of the process. Only those interface implementations residing in VTABLE
regions can be called. Net effects of this limitation are two:
1. Only RPC services, which were intentionally provided as such, can be used
   to perform legitimate RPC call. No random data put together which resemble
   pointer-to-function array can be used as a RPC service.
2. Presence of VTABLE address in some particular process' VTABLE region
   established relationship between RPC service and owning process. This is
   also important from memory protection perspective, as it allows kernel to
   determine, how to set up memory protection during the RPC call.

@page dev_env Development environment

CMRX itself is built on top of CMake build system. CMRX only contains the
minimum set of tools, services and rules to make things actually happen, so
unlike others offers in the real-time operating system market, CMRX does not
provide it's own HAL nor drivers. This allows CMRX to be integrated with pretty
much any CMSIS-compliant HAL and to support broad range of target
microcontrollers.

From the developer's point of view, every process is composed of single library,
which is then linked to the final binary.


