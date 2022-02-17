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
==================================

From high level perspective, RPC interface works in a way similar to C++
objects. There is one or more classes declared. Those classes, in case of C++
language, declare both internal structure and external interface (let's ignore
internal private interface for now). Program can, and often will, create
instances of these classes. Each instance of given class contains it's own
private copy of data, but all instances of single class share the very same
external interface. Any caller of this interface will rely on the way how this
interface is defined, but shall not rely (nor be able to access) on internal
structure of class data.

Same is true for RPC mechanism in CMRX. Applications in CMRX define interfaces,
which can be used to access objects from outside of the application, which
defined them. While interfaces **have** to be defined statically, similarly to
C++, objects don't need to be.

The difference here is in use of words *interface* and *class*. In C++, class
definition covers both methods and data. CMRX doesn't work with classes, rather
with interfaces. Interfaces only describe methods used to access objects and
form the public facade of object. Internal definition of data is then completely
isolated from it's public interface.

RPC interface
-------------

RPC interface is a structure, which lists pointer-to-function kind of members.
This is a typical way of writing "C classes". There are no special requirements
for internal organization of this structure except of typical best practices in
writing reusable interfaces.

There are limitations given by RPC mechanism in CMRX though. Any method of RPC
interface has to have 1 up to 5 arguments. First arguments **must** be pointer
to instance. This semantics is same as used in Python methods. To simplify
things, you can use all caps `SELF` to fullfil that need. Remaining four
arguments can be of any scalar type. Method can return any scalar type, but does
not need to return anything.

Method entry in class may look like:

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void (*method_name)(SELF, int arg);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

RPC object
----------

RPC object holds the data, RPC interface provides access to. It is plain C
structure with only one requirement. First member of this struct **must** be a
pointer to RPC interface, which provides access to this object and this member
**must** be called `vtable`. Amount, types, structure and order of remaining
items is completely up to application developer. Even removal or reordering of
items within structure won't compromise binary compatibility, as long as your
interface can cope with it.

Becoming a RPC server
=====================

In order for some application to define a RPC server, it has to do quite a few
things:

Declare interfaces
------------------

This is done by creating RPC interface structure for each interface application
wants to provide. This is a two-step process. First, you have to declare the
structure of the interface. This is done by providing type declaration for
structure containing pointers to functions. Application probably wants to
provide this declaration in some header file, which can be included by potential
caller. This is basically the only publicly visible part of RPC interface.

Declare object structure
------------------------

This is done by creating RPC objects. These are, similarly to RPC interfaces,
structure declaration, but unlike RPC interfaces, these may remain private to
the application. Even if caller knew internal structure of RPC object, it is not
accessible from it's address space anyway. Here application developer may
declare as many RPC object types as he needs for the job to be done. Multiple
object types may use the same interface, if appropriate.

Define interface implementation
-------------------------------

Publicly facing RPC interface needs to have it's implementation. This is done in
two steps. First you have to provide the actual implementations of functions
having prototypes suitable for use in RPC interface. This means that you have to
provide one function for each entry in every RPC interface.

Then you have to map those functions to entries in RPC interface. This is done
simply by creating a variable, whose type is RPC interface type used while
declaring RPC interface. This variable is initialized to reference to functions,
which implement methods of RPC interface.

One special requirement of this variable is, that it has to be prefixed with
keyword `VTABLE`. This keyword serves the purpose of putting such variable into
VTABLE "redion" of an application. Only RPC interfaces residing in VTABLE
regions can be called. Net effects of this limitation are two:
1. Only RPC interfaces, which were intentionally provided as such, can be used
   to perform legitimate RPC call. No random data put together which resemble
   pointer-to-function array can be used as a RPC interface.
2. Presence of VTABLE address in some particular application's VTABLE region
   established relationship between RPC interface and owning process. This is
   also important from memory protection perspective, as it allows kernel to
   determine, how to set up memory protection during RPC call.
