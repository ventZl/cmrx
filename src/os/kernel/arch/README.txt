Kernel portability layer
========================

Header files in this directory describe the functional interface of the 
architecture-independent portion of the kernel towards the architecture support 
layer. Any port needs to provide implementation of these functions.

Aside from these functions, kernel will expect several files existing in porting
layer include directory (cmrx/include/cmrx/arch/<architecture>/<hal>/arch):

corelocal.h 
-----------

This file shall define two symbols:

coreid() - this symbol shall evaluate to the ID of current CPU. On single-core systems
           it may be provided as a macro evaluating to constant 0
OS_NUM_CORES - this symbol shall provide count of CPU cores the current system has. It 
               may be provided as a macro.

mpu.h
-----

This file shall define one type:

struct MPU_Registers - it should contain enough space that state of one MPU region can
                       be saved into.

sysenter.h
----------

This file shall define two objects:
__SYSCALL - alias of attributes that are needed for function to act as a syscall entrypoint
            from the userspace. It may be left empty if architecture has no special needs.

__SVC() - this shall evaluate to single-argument function-like object that performs kernel
          call. It can be defined as a macro evaluating to inline code. Argument denotes the
          syscall ID being called.

