option(CMRX_ARCH_SMP_SUPPORTED "Architecture supports SMP and project is using it" OFF)
option(CMRX_OS_NUM_CORES "Amount of cores present in CPU package" INT:1)
option(CMRX_UNIT_TESTS "Enable build of kernel unit tests" ON)
option(CMRX_KERNEL_TRACING "Enable tracing of kernel events" OFF)
set(OS_STACK_SIZE 1024 CACHE STRING "Stack allocated per thread in bytes")
set(OS_THREADS 8 CACHE STRING "Amount of entries in the thread table")
set(OS_PROCESSES 8 CACHE STRING "Amount of entries in the process table")
set(OS_STACKS 8 CACHE STRING "Amount of stacks allocated")

# List of CMake options that are transferred into nested build
set(CMRX_ALL_OPTIONS
    CMRX_ARCH_SMP_SUPPORTED
    CMRX_OS_NUM_CORES
    CMRX_KERNEL_TRACING
    OS_STACK_SIZE
    OS_THREADS
    OS_PROCESSES
    OS_STACKS
    )
