@page overview Overview

This page provides an overview of basic concepts in CMRX environment. You shall
make yourself familiar with all the concepts listed here in order to understand
how CMRX environment behaves and how to achieve certain goals.

@subpage concepts describes model of execution under CMRX

@subpage mem_model outlines how memory is organized and what can be accessed and how

@subpage exec_model provides information on how userpace code is selected for execution
under CMRX scheduler.

@subpage rpc_intro introduces the RPC mechanism

@subpage txn_intro describes how transactional processing within kernel works

@subpage dev_code_organization outlines how the code has to be organized in the project

@subpage dev_env describes the how the build environment and integration with vendor's SDK
works

@subpage hal_integration outlines how SDK libraries and HALs are integrated into
CMRX-based project.


@page concepts Basic concepts
If using CMRX, there are some basic concepts enforced by the kernel itself.

Application code is organized into processes.

\dot "Organization of CMRX runtime" width=720px
digraph D {
    subgraph cluster_all {
        label = "Application Firmware";

        node [shape = record];

        isr_1 [ label = "ISR 1" ];
        isr_2 [ label = "ISR 2" ];
        core [ label = "Kernel" ];
        thread_1 [ label = "Thread 1" ];
        thread_2 [ label = "Thread 2" ];

        subgraph cluster_kernel {
           label = "Privileged";

            core;
            isr_1;
            isr_2;
        }

        subgraph cluster_userspace {
            label = "Userspace";

            subgraph cluster_process_1 {
                label = "Process 1";

                thread_1;
            }

            subgraph cluster_process_2 {
                label = "Process 2";

                thread_2;
            }
        }
    }
}
\enddot

Process is a collection of accessible 
memory, threads and RPC services. Any userspace code has either run as ISR handler, or
be launched from userspace thread, which is created in context of userspace process.
There is no way to run kernel-privileged code outside interrupts. Currently, processes
have to be statically declared at compile time and it is not possible to create new 
processes during runtime.

\dot "The anatomy of CMRX process" width=720px
digraph D {
    subgraph cluster_process {
        node [shape = record];
        stack_1 [ label = "Stack", rank=2 ];
        stack_2 [ label = "Stack", rank=2 ];
        initialized_data [ label = "Initialized data" ];
        bss [ label = "BSS" ];
        shared_memory [ label = "Sharable data" ];
        rpc_services  [ label = "RPC services" ];

        label = "Process";

        subgraph cluster_thread_1 {
            label = "Thread 1";

            stack_1;
        }

        subgraph cluster_thread_2 {
            label = "Thread 2";

            stack_2;
        }

        initialized_data;
        bss;
        shared_memory;
        rpc_services;
        bss->stack_1 [style = invis];
        bss->stack_2 [style = invis];
    }
}
\enddot
Thread is an execution context within process. Thread has its private stack, priority,
and hosting process. Threads can be auto-started on system startup after processes
are created. This effectivelly allows fully-static operation. Threads can also be created 
during runtime. It is not possible to inject a thread into foreign process.

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

@ref rpc_intro services are the only supported means of inter-process communication. While threads
belonging to the same process all share the same memory, there is no means of sharing memory
belonging to any other process unconditionally. One has to use RPC to execute code within 
context of other process.

@page mem_model Memory Model
Code running in the context of process in CMRX environment is limited by memory protection.
Therefore the application (or, better: process) can only access resources, which
belong to it. In CMRX, the process is the governing entity when it comes to what
resources can be accessed from within specific execution context. A process is thus a 
container which groups all accessible resources living in the user space.

Code and read-only data
-----------------------

All code, which doesn't belong to kernel itself (with some very specific
exceptions) has to belong to one of processes declared at build time.
Process has to have name and is given access to certain memory areas.

Due to limitations of Cortex-M hardware, whole flash region is marked as read +
execute. It would be extremely difficult to provide working implementation of
memory protection which would allow use of library functions if process' code execution was 
explicitly allowed by the memory protection. And it would probably impossible to do that statically. This
design decision has effect you have to be aware of as a developer: **every code
can read everything in flash, including any possible secrets stored there**.

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
conditions, access rules to shared variables are same as for static
variables. All the code of all compilation units belonging to the owning
process can access all the shared variables.

The situation changes, when a \ref rpc_intro is performed. While remote
procedure call is in flight, called procedure has access to shared memory of
calling process. This is effectively the only way of passing larger amount of 
data between processes.

Stack
-----

Stack is automatically allocated for each and every thread running in system.
Stack is thread-private and even no two threads of same process cant access each
other's stack. Stack is automatically reclaimed by the kernel, once thread
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

@dot "Hierarchy of RPC components" width=720px
digraph D {
    node [shape="box"];
    interface [ label = "RPC Interface" ];
    implementation_1 [label = "Implementation 1"]
    implementation_2 [label = "Implementation 2"]
    interface;
    service_1 [ label = "Service 1" ];
    service_2 [ label = "Service 2" ];
    service_3 [ label = "Service 3" ];
    
    subgraph cluster_process_1 {
        label = "Process 1";

        implementation_1;
        service_1;
        service_2;
    }

    subgraph cluster_process_2 {
        label = "Process 2";

        implementation_2;
        service_3;
    }

    implementation_1 -> interface;
    implementation_2 -> interface;
    service_1 -> implementation_1;
    service_2 -> implementation_1;
    service_3 -> implementation_2;
}
@enddot

From the server perspective, the service instance is a fully defined structure,
which holds service data and contains pointer to the list of callable service methods.

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

RPC interface
-------------

RPC interface is a structure, which lists pointer-to-function kind of members.
This is a typical way of writing "C classes". There are no special requirements
for internal organization of this structure except of typical best practices in
writing reusable interfaces.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
#include <cmrx/rpc/interface.h>

struct RPC_interface {
    void (*method_1)(INSTANCE(this), int arg1);
    int (*method_2)(INSTANCE(this), int arg1, int arg2);
    void * (*method_3)(INSTANCE(this) /* no arguments */);
};

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

There are limitations given by RPC mechanism in CMRX though. Any method of RPC
interface has to have 1 up to 5 arguments. First arguments **must** be pointer
to instance. This semantics is same as used in Python methods. Remaining four
arguments can be of any scalar type. Method can return any scalar type, but does
not need to return anything.

RPC service
------------

RPC service holds the data, RPC service provides access to. It is plain C
structure with only one requirement. First member of this struct **must** be a
pointer to the RPC service, which provides access to this object and this member
**must** be called `vtable`. Amount, types, structure and order of remaining
items is completely up to application developer. Even removal or reordering of
items within structure won't compromise binary compatibility, as long as your
service can cope with it.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
struct RPC_service {
    const struct RPC_interface vtable;
    long long service_data;
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Becoming a RPC server
---------------------

In order for some application to define a RPC server, it has to do quite a few
things:

Declare interfaces
------------------

Very first step is to define the interface. Interface is a structure visible
both to the server and the client, which defines what actions some type of service
supports. Interface is purely procedural and does not support publishing of data
members. Interface has a form of structure containing pointer-to-function
members. Interfaces shall be as abstract as possible, creating contracts between the
caller and the callee rather than describe manipulation methods specific to one single
interface. This facilitates interface reuse. If your service has possible actions similar
to some other service, then both services shall implement the same interface. If they do
then these services are interchangeable. Caller can call an instance of both without
actually knowing what specific service it is calling or that they are in fact different.

This step can be skipped if there is suitable interface available somewhere in your
project. You can reuse it rather than defining your own interface.

Declare services
----------------

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

Next, it is necessary to provide implementation of all functions declared in the
interface this service implements. Once these functions are implemented, they can
be put together into interface implementation table. This is an instance of
interface, where function pointers point to actual functions just created.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}

#include <cmrx/rpc/implementation.h>
IMPLEMENTATION_OF(struct RPC_service, struct RPC_interface);

static void service_method_1(INSTANCE(this), int arg1)
{
    /* ..... */
}

static int service_method_2(INSTANCE(this), int arg1, int arg2)
{
    /* ..... */
}

static void * service_method_3(INSTANCE(this))
{
    /* ..... */
}

VTABLE struct RPC_interface service_vtable = {
    service_method_1,
    service_method_2,
    service_method_3
};

struct RPC_service service = {
    &service_vtable,
    0x42
};
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If any interface provides some function, then it is mandatory to implement it.
No pointer in interface can point to NULL. Doing so will crash any caller which
attempts to call such interface method.

Instances of interfaces have to be marked by special keyword @ref VTABLE.
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

Calling RPC service
-------------------

Once RPC service has been established having its own implementation it is possible to
perform calls of this service. Service instance lives in the address space of the process,
which instantiated it. It is not possible to place the service instance into some
globally-accessible place as no such space exists. It is not even needed. All that any
potential caller needs is the address of the service. Call of this service can then be
performed using the following code:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
int retval = rpc_call(service_ptr, method_2, 0x42, 0xF00F);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This call will perform the call of method_2 from within service using CMRX RPC mechanism.

Advantages of RPC mechanism
---------------------------

Why mechanism so complicated?

Despite looking complicated, this mechanism has a few advantages over directly calling
functions, or calling functions via pointer-to-function table indirection. If we first
consider the simplest case of calling specific service functions via their names directly,
such as:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
int retval = service_method_2(service_ptr, 0x42, 0xF00F);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Then there is an obvious difference, that in the latter case, you are explicitly stating
the method to call. While using the RPC mechanism, you are only stating its name. So the
service is able to provide whatever specific function that implements the requested method
for this specific kind of service. Calling client and called server are tightly bound to
perform one specific action.

Another way of calling service methods in a polymorphic way is to use "object oriented C"
approach and call the method like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
int retval = service_ptr->vtable->method_2(service_ptr, 0x42, 0xF00F);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This approach is much closer to what RPC call actually does with a few little, yet
important differences. Even if it looks like the actual function executed when `method_2`
is called can be changed, it isn't absolutely true. There is still rather high degree of
binding between the interface and the service here. The function, whose address is used to
initialize the vtable structure must match the type and count of arguments in the vtable
type declaration. This will force you to use method with the following prototype:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.c}
int (*method_2_t)(RPC_service * this, int arg1, int arg2);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

This works as long as you have one service and one interface. There is direct relationship
between RPC_interface structure whose members type their first argument as `RPC_service *`.
If you attempt to create an interface usable with multiple services, you will face type
incompatibility problems. Either you'll have to typecast functions during the
initialization of RPC_interface instance, or use some generic type for `this`, such as
`void * this`. Both will clutter the code with typecasts and will throw part of type
safety as typecasts will be explicit and can contain typos or other mistakes.

Another problem of both approaches is, that the function which ends up being called one
way or another must reside in the same process as the caller. You can't call function from
within another process as it will crash in the moment it tries to access it's own
process' memory.

RPC call mechanism deals with both problems. Syntactic sugar around the macro @ref INSTANCE() 
causes that whenever an interface is declared, its `this` pointer is not typed. It
effectively behaves as `void * this`. On the other side, whenever you provide an
implementation of the interface, you have to provide a service - interface pair. When you
do this, the check, if the intended service actually provides intended interface will
happen. This way, typos are avoided. Attempt to implement an interface for service it
isn't providing it will cause build error. Another effect is that use of macro INSTANCE()
during the implementation of the interface will use the correct type expected by this
specific implementation.

From the client's perspective, the actual type of service data structure is not important.
Client handles structure instance as an opaque pointer during runtime. It never accesses
it as it resides in other process' memory. All the preparatory work to implement the RPC
call is done during the compile time. Here, the type checking is done, so that the type
and amount of arguments to the call is checked. Compilation will fail if there is a
mismatch. Compilation will also fail if there is an attempt to call non-existent RPC
method.

Then, during the runtime, when RPC call is performed, CMRX kernel will make necessary
adjustments of the runtime environment to allow the called method to access the memory
region of owning process.

@page exec_model Execution Model

As long as memory is concerned, the entity governing access is a process. As long as
scheduling is concern, the governing entity is a thread. CMRX contains prioritized,
multi-threaded, preemptive scheduler. As of now, this scheduler is not SMP-capable.

Scheduler
---------

Scheduler of CMRX RTOS offers 256 different priorities of threads, with priority 0 being
the highest and priority 255 being the lowest, reserved for the idle task use. Scheduler
is prioritized, so whenever a thread has to be selected for execution, the thread with
the highest priority (the lowest numerical value of priority) is selected that is ready to
execute.

Priorities are assigned per-thread, not per-process. So it is possible for one process to
have multiple threads having different priorities. Threads with higher priorities might be
used to handle external events, while threads with lower priorities might be used to
crunch asynchronous work.

Scheduler is preemptive, which means that whenever there are two threads ready to execute
having same priority, they will share the CPU time. Scheduler will periodically switch from
one thread to another so that both threads can run "simultaneously". This is something to
consider when designing event handler thread priorities. If two event handler threads have
the same priority, then it may happen that arrival of event handled by some of these
threads, while other thread is already running may preempt one thread by another. This may
introduce non-determinism or unacceptably long latencies.

There is no per-process prioritization. Whenever a thread has to be selected, all threads
from all processes are considered. Process switch occurs automatically if the next thread
to be executed belongs to different process than the thread being swapped out. There is no
fairness in CPU time allocated to threads or processes. CPU time is always allocated to
thread(s) with highest priority that are runnable. If there is more than one such thread,
then CPU time is divided between these processes evenly.

Interrupts
----------

All the code running in the userspace, even the code of threads with priority 0 has lower
priority than any interrupt that may happen. This way any code registered as an interrupt
handler will preempt any thread running. Device interrupt handlers have priority higher
than the kernel service handler. The effect of such setup is, that hardware
interrupts will preempt even running kernel system call. This has the consequence, that it
is not possible to call system calls from within interrupt service routines. There is
special set of @ref api_isr minimalistic API which allows to run limited subset of kernel 
services from within interrupt service handler. The goal of this design is to make ISRs as 
lean as possible to they won't disturb thread execution much.

Only perform time-sensitive work in the context of an ISR and defer all remaining work to
designated thread.

@page dev_code_organization Organization of the Code

Any CMRX-based project has to follow certain rules of code organization in order to build
and run successfully. Typical project organization is shown on the figure below:

@dot "Hierarchy of source files" width=720px
digraph G {
    node [shape = "box" ];
    main_c [label = "main.c" ];
    utils_c [label = "utils.c" ];
    process_1_c [ label = "process_1.c", rank=2 ];
    process_2_c [ label = "process_2.c", rank=2 ];
    service_1_c [ label = "service_1.c", rank=2 ];
    service_2_c [ label = "service_2.c", rank=2 ];
    system_c [ label = "system.c", rank=3 ];
    startup_c [ label = "startup.c", rank=3 ];
    one_c [ label = "...." ];
    two_c [ label = "...." ];

    subgraph cluster_firmware {
        label = "firmware.elf";

        main_c;
        utils_c;

        subgraph cluster_app_1 {
            label = "libprocess_1.a";

            process_1_c;
            service_1_c;
        }

        subgraph cluster_app_b {
            label = "libprocess_2.a";

            process_2_c;
            service_2_c;
        }

        subgraph cluster_hal {
            label = "libsdk.a";

            startup_c;
            system_c;
        }

        subgraph cluster_cmrx {
            label = "licmrx.a";

            one_c;
            two_c;
        }

    }

    process_1_c -> startup_c [style = invis ];
    one_c -> process_2_c [style = invis ];
    one_c -> process_1_c [style = invis ];
}
@enddot

Figure shows several C files organized into a hierarchy of libraries.

First, there is a group of C files represented by the usually present `main.c` file, which
provide some basic MCU configuration and then start the OS kernel. These files might
contain stuff such as GPIO configuration, configuration of system clock sources and
possible static initialization of peripherals, such as their clock source and mapping to
GPIO pins. Main rule is that things done here shall not deal with initialization of
processes or threads. Another content of C files which are compiled directly with the
final executable and don't belong into any process are sources containing shared library
routines. These files can contain code that will be accessible by all processes but can't
contain any data as the data defined in modules not belonging to any process won't be
accessible from the userspace.

Next, there is a group of files belonging to the HAL used. Here, the actual form might
vary depending on HAL used. Minimalistic CMSIS-based example is depicted, which usually
provides at least two modules: startup.c and system.c. Former contains startup code which
ultimately leads to main() being executed while latter contains implementation of some
basic system code, usually basic SysTick and NVIC routines. The organization of these
files is governed by the HAL used and might not be organized into library. Some SDKs
generate these files dynamically, while others provide feature-rich library containing
fully-equipped SDK.

Then there are multiple libraries. Each library represents one process. All global
variables in all C files linked into `libprocess_1.a` are accessible from all the code
which runs in any thread owned by the `process 1`. Note that all code is executable at all
times, so all the data of `process 1` is visible not just from code of process own C files,
but also from the code provided by the `utils.c` source file. What truly matters is, in
what context the code runs. If the code will be running as part process' 1s thread
execution, then it will be able to access its data. If the code will be running in the
thread owned by the `process 2`, then no data of `process 1` will be visible regardless of
where the code executing is stored. One process can group as many C files as necessary and
they all will be able to access each other's data without any specific precautions.

Last part is the CMRX kernel itself. It is linked as a library and contains all the
relevant portions of the kernel itself. None of its data is accessible from any process.

@page txn_intro Transactional Processing

One of important roles of any kernel is to keep things consistent at all times. This is 
rather trivial task even in a multi-threaded environment as long as your kernel processing
does not get preempted. This normally does not happen, unless your ISRs are calling kernel
services. Once they start doing so, the kernel code may get interrupted at any time with 
routines modifying the current kernel state.

Another similar, but not entirely the same situation happens if your kernel is running on 
multiple CPUs in SMP arrangement. Two CPU cores may execute kernel code in parallel.

While the latter situation can easily be resolved by using so called "big kernel locks" - 
locks which ever only allow one core to enter critical section - it won't help with interrupts.

If you allow interrupt to be handled while in critical section and this interrupt decides to 
enter this critical section, you get a deadlock situation. This is due to the priority inversion.
Kernel code which is running inside ISR with lower priority holds a lock which prevents a code 
from higher priority ISR to proceed. Due to the priorities the code already holding the lock 
will never be able to proceed and everything hangs.

If you disable interrupts for the duration of the critical section, then the worst-case latency
is hurt as you need to account for the longest-possible critical section blocking your interrupt 
from happening.

From critical section to transaction
------------------------------------

The approach selected for CMRX is to split critical section into two distinct parts. This split
roughly aligns with transactional processing, thus this term was chosen to describe the behavior.

Critical sections also have three steps:
1. Lock the mutex and enter the critical section
2. Do whatever calculations needed in isolation and then modify the shared data structures
3. Unlock the mutex leaving critical sections allowing others to proceed

Step 1 and 3 above are usually ignored as they are considered to happen immediately if no other
code is already in the critical section.

You either get deadlocks in your interrupt service routines if you don't disable interrupts in 
critical sections, or your interrupt latency gets quite bad if you do.

To fight these two problems, different semantics is used. Code that would normally reside in 
critical section is said to reside inside transaction. Then the steps are:

1. Start the transaction obtaining transaction identifier
2. Do whatever calculations needed before you are ready to write into shared data structures 
3. Commit transaction
4. If successful, then write changes do the shared data structures
5. Finish the transaction

There are two additional steps. The main difference between critical section and transactional
processing is that while step 2 of critical section happens inside the critical section, thus
code has exclusive access to the shared resources, step 2 of transactional processing happens
in normal mode of operation. This code can be interrupted or another code running on another
core(s) can modify the data while this code runs.

Normally that would lead to race conditions. With some caution in writing the code, this does 
not happen here. The step 3 of transactional processing makes the difference.

When the code makes an attempt to commit the transaction created in step 1, the transaction
engine checks, if there was another transaction committed in between this transaction was 
created and the commit attempt has been made. It does not matter if this transaction happened
inside ISR or on another core.

If another transaction has been **committed** while the computation of this transaction 
computation ran, then the computation is considered inconsistent. It is then up on the calling 
code to decide on what to do now. In some cases, transaction can be dismissed entirely. In other
cases it has to be computed again.

If there was no conflict and commit is allowed, then critical section is entered and the code 
has exclusive access to the resources and can modify the shared kernel data. Here the **atomicity**
is ensured.

Transaction caveats
-------------------

Unlike transactional databases, the transactional processing inside the kernel is not fully 
isolated. While computation of transactions happens outside of critical sections, it may happen 
that code running one core will read the data while critical section running on another core 
will update it.

This could be avoided by processing in isolation, creating a local copy of data being processed
upon starting transaction. This would be extremely expensive as such kind of conflicts only 
happens rarely. In most cases, the effect of modification done inside the transaction only 
affects the local core. Thus the decision here is to make sure that changes made while 
committing transactions are made in a way that there won't be any transient hazard state 
generated.

Transient hazard state happens when the current state of data in shared kernel structures,
if interpreted by another piece of code, could lead to an unexpected and unwanted behavior.

Transaction engine properties
-----------------------------

The transaction engine provides level of guarantees roughly at the level "read committed".

There are two types of transactions supported:
* read-only transactions - the sole purpose of these transactions is to make sure no other 
  transaction has been committed while the computation was performed. This ensures that whatever
  data were read were in consistent state all the time. 
* read-write transactions - this type of transaction ensures that all writes to the shared
  context were made atomically and that no other read-only transaction will process these data 
  in an inconsistent manner without knowing it.

Guarantees offered:
* Atomicity of writes - changes written are written atomically, no two transactions will write their
  changes simultaneously. There is no guarantee that reads will be atomic. See isolation below.
  There can be only one transaction committing its changes at any time in the whole system.
* Consistency awareness - computations made within transactions are let know if the shared state could 
  become inconsistent. If this hint is honored and changes are written in a hazard-free way, then
  consistency of data is guaranteed even in read-only transactions. There can as many read-only 
  transactions running in parallel as needed. Committing any of them won't abort any other 
  concurrent read-only or read-write transaction. Yet if there is read-write transaction committed
  while there are read-only transaction opened, all of them will be aborted.

Guarantees not offered:
* Isolation - read-only transactions don't access the data isolated from other (mostly read-write)
  transactions. There may happen a commit at any time of processing read-only transaction. Reads
  are not blocked while commit is writing modifications. Thus the code modifying the data must
  ensure there are no data hazards.
* Durability - this guarantee is not provided as it makes no sense. The current state of the
  kernel is volatile. No persistence is provided.

@page dev_env Development Environment

CMRX is using CMake build system. There are several entities which are visible from the
project which integrates CMRX in their buildsystem:

* CMRX CMake module - this module defines several functions that wrap creation of
  libraries and executables with post build events and calculate certain properties.

* CMRX source and header directories - here, the code and headers of CMRX are stored. CMRX
  is distributed as a source and is compiled during the build of the firmware. Build of
  CMRX results in creation of library named `os` which you have to link to the final
  executable. Include paths to headers are automatically propagated to each target which
  links to the `os` library.

* Linker script adjustment tool - for seamless operation of memory protection, it is 
  necessary to calculate alignment values for private data of each process. This mechanism
  takes the original linker script and injects per-process include blocks into it. These
  blocks are then adjusted based on map file analysis. This altered linker script is then
  used to link the binary.

CMake functions
---------------

While firmware built using CMRX is pretty much standard binary, there are certain tasks
done after the binary is built. Similarly, every CMRX process is wrapped into fairly
standard static library, yet there are some tasks done after the library is built. In
order for things to work correctly, CMRX is also maintaining a set of properties bound to
either firmware images or process libraries. There are three main CMake functions
available for use:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
add_firmware(<firmware name> <source file(s)>)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use this function in place of `add_executable()` to define firmware binary. Adding binary
this way will automatically enable map file creation, apply patched linker script and add
post-build actions which automatically recompute alignment constants. It also generates
properties needed to ensure reliable operation.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
add_application(<application name> <source file(s)>)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use this function in place of `add_library()` to create a static library which contains
all the process data and code. Creating process this way will automatically add some post
build events which perform sanity checks and create linker script includes.

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
target_add_applications(<firmware name> <application name(s)>)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Use this function to link application libraries to the firmware binary. This command
effectively wraps `target_link_libraries()` command, while doing some sanity checks and
generating post-build events to ensure that the linker script is always up to date.

Source and header directories
-----------------------------

CMRX is distributed as a source and it has to be compiled with your application in order
to build runnable firmware. To accomplish this you have to use `add_subdirectory()`
command to add CMRX sources into your project. By doing so, several new targets will be
defined:

* library `os` - this library wraps all the compiled code of the CMRX kernel. It is
  internally composed of two parts: the portable part and the architecture port. The
  architecture port is handled automatically. Library propagates include paths into every
  target you link against it. Therefore you don't have to set up include paths to be able
  to include CMRX headers explicitly.

* library `stdlib` - this library contains functions that call system calls. These
  functions are callable from processes. See @ref api to learn on what functions are
  available.

* library `aux_systick` - this is an auxiliary library which implements SysTick-based
  timing provider. This optional library can be used to quickly bootstrap the kernel
  before more comprehensive hardware- and task-specific timing provider is developed.

Linker script adjustment tool
-----------------------------

This tool makes the use of memory protection seamless. The main purpose of this tool is to
recalculate alignment of data regions of each process based on its size. This is
necessary, because memory protection units on most microcontrollers have a limitation,
that a region which has access rights defined by MPU must have it's base address aligned
at least to its size. So if the region is 256 bytes large, it has to start at address
aligned to 256 bytes. Due to this, the size of data segments has to be determined after
build and then new alignments have to be determined. To save the RAM, regions are ordered
descending based on their alignment.

One side effect of this mechanism is, that sometimes, the binary has to be linked twice.
This happens when the size of data block causes that the alignment value used to link it
is not valid anymore. In such case, you have to link the binary again using new, up to
date linker script. If linker script adjustment tool detects this case, it will delete the
firmware binary. This behavior is to prevent use of incorrectly linked binary which won't
run.

@page hal_integration Integrating vendor SDKs

As mentioned before, CMRX does not provide any OS-specific HAL or SDK. You have to
integrate vendor-provided HAL. This way, the use of CMRX is as non-intrusive as
technically possible. CMRX itself - being a microkernel - uses only minimalistic set of 
peripherals, almost exclusively only peripherals residing on Cortex core itself. Only
architecture dependent part of the kernel code is ever using these peripherals. This part
is always written with some HAL in mind. For example the default ARM port is written using
CMSIS standard. CMSIS standard is quite broad but minimalistic needs of CMRX kernel can
usually be satisfied by the basic CMSIS device header. This header is often present in
vendor SDK for a microcontroller in some form usable by the CMRX.

Second part usually present in the vendor SDK is the linker script for your target device
which declares base addresses of RAM and FLASH and defines regions for code, data, bss and
others. This script is needed to link the binary correctly. CMRX needs this scriupt to be
manipulated in order to enable memory protection.

Third part of vendor SDK are the startup and system source files. They might or might not
be present, or may be present in slightly different organization. Here vendors are
creative and mostly ignore CMSIS standard.

As long as the CMRX kernel itself is concerned, this is all that is needed from the vendor
SDK. Your application's need will usually be bigger as you might want to use peripheral
drivers and utility libraries provided by the SDK. While the range and way of using these
vary from project to project, CMRX does not deal with these at all. It is up to integrator
to include them into project.

CMSIS helper
------------

To make integration of CMSIS HAL for needs of the CMRX easier, there is small CMake module
which helps to find various CMSIS components and set up defines. This module is called
`FindCMSIS.cmake`. This module will determine include paths for core CMSIS headers and try
to find startup and system components of CMSIS, if available. It will copy the linker file
to location CMRX expects to find one.

The basic use of `FindCMSIS` is like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~{.cmake}
set(CMSIS_ROOT <path to where CMSIS is located in vendor SDK>)
set(DEVICE <device name recognized by the vendor SDK>)
set(CMSIS_LINKER_FILE <location of the linker file>)
include(FindCMSIS)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

If necessary CMSIS components are found, then following is done:

* library `cmsis_core_lib` is created. This is an interface library that propagates
  include paths to all headers of CMSIS core. This ensures that the CMSIS headers can be
  included into CMRX source. ARM port based on CMSIS HAL expects this library to exist
  while compiling.
* file `RTE_Components.h` is creates in `CMAKE_BINARY_DIR`. This file contains an alias to
  device file as per CMSIS specs, so the CMRX kernel code is device-independent. ARM port
  based on CMSIS HAL expects this file to exist.
* linker file is copied into file named `gen.<DEVICE>.ld` into `CMAKE_BINARY_DIR`. This is
  where CMRX expects to find the device linker script. If linker script provided in
  variable `CMSIS_LINKER_FILE` does not exist, then FindCMSIS will fail with error.
* variable `CMSIS_SRCS` is defined which contains list of files found that compose the
  CMSIS core. Only files named according to CMSIS specs are detected. These files are not
  handled automatically, rather it is the integrator's task to handle them in a way the
  vendor SDK is designed to do.
