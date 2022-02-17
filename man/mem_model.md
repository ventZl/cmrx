\page mem_model Memory Model

Applications running in CMRX environment have memory protection use enforced.
Therefore the application (or, better: process) can only access resources, which
belong to it. In CMRX process is the governing entity, when it comes to what
resources can be accessed from within specific execution context.

Code and read-only data
-----------------------

All code, which doesn't belong to kernel itself (with some very specific
exceptions) has to belong to one of applications declared at build time.
Application has to have name and is given access to certain memory areas.

Due to limitations of Cortex-M hardware, whole flash region is marked as read +
execute. It would be extremely difficult to provide working implementation of
memory protection which would allow use of library functions if code was under
memory protection. And it would probably impossible to do that statically. This
design decision has effect you have to be aware of as a developer: every code
can read everything in flash, including any possible secrets stored there.

When it comes to RAM, the situation is completely different. Any application can
only access RAM claimed by variables being defined by application's code.
Application RAM region is generally divided into three separate sections:

Static variables region
-----------------------

This region is composed of all variables defined in all compilation units
belongin to some specific application. It is sufficient to define a variable in
compilation unit, which ends up being compiled as a result of add_application()
call and that variable will be part of application's memory space. Such memory
can only be accessed by threads running in the context of owning process. This
means, that all the code of all compilation units of given application can
always access all such memory.

You don't need to treat those variables in any special way in order for your
code to work properly. If variables are non-static (i.e. globally visible), you
can access them from every compilation unit belonging to owning application.

Shared variables region
-----------------------

This region is composed of variables defined in compilation units belonging to
some specific application, which has their definition prefixed by keyword
`SHARED`. Doing so will relocate given variable into shared region. Under normal
conditions, access rules to shared variables is the same as for static
variables. All the code of all compilation units belonging to the owning
application can access all the shared variables.

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

There is also one region, which is custom-defined by application designer.
Common use case of this region happens, when the application is a
device driver of some kind and needs to access HW directly. As device drivers
are just an applications in microkernel world, any such application needs to gain
access to memory region which allows control of given peripheral.
