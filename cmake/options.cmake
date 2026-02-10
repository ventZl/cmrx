option(CMRX_ARCH_SMP_SUPPORTED "Architecture supports SMP and project is using it" OFF)
option(CMRX_OS_NUM_CORES "Amount of cores present in CPU package" INT:1)
option(CMRX_UNIT_TESTS "Enable build of kernel unit tests" OFF)
option(CMRX_KERNEL_TRACING "Enable tracing of kernel events" OFF)
option(CMRX_CLANG_TIDY "Enable linting using Clang-tidy" ON)
option(CMRX_KERNEL_TRANSACTION_VERIFICATION "Enable checking for nested transaction commits" OFF)
option(CMRX_IDLE_THREAD_SHUTDOWN_CPU "Idle thread stops CPU" OFF)
option(CMRX_RPC_CANARY "Enable RPC canaries" OFF)
option(CMRX_MAP_FILE_WITH_EXTENSION "MAP file contains full name of binary with ELF extension (.elf.map)" OFF)
option(CMRX_CLANG_TIDY_LIBC_PATH "Path to standard C library used while linting" /usr/include)
option(CMRX_HIL_TESTING "Enable build of HIL tests and sanity check of HIL infrastructure configuration." OFF)
option(CMRX_INTEGRATION_TESTS "Enable build of integration tests. Implies CMRX_HIL_TESTING=ON!" OFF)
option(CMRX_HIL_TESTING_SKIP_OPENOCD "Skip OpenOCD initialization during integration tests." OFF)
option(CMRX_USE_FAST_HASH "Use fast hashing function at expense of hash quality" OFF)
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
