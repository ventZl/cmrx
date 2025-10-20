option(CMRX_ARCH_SMP_SUPPORTED "Architecture supports SMP and project is using it" OFF)
set(CMRX_OS_NUM_CORES 1 CACHE STRING "Amount of cores present in CPU package")
if (CMAKE_HOST_WIN32)
    set(UNIT_TESTS_DEFAULT OFF)
else()
    set(UNIT_TESTS_DEFAULT ON)
endif()
set(CMRX_PLATFORM "generic" CACHE STRING "Specific platform within architecture to build for")

option(CMRX_UNIT_TESTS "Enable build of kernel unit tests" ${UNIT_TESTS_DEFAULT})
option(CMRX_KERNEL_TRACING "Enable tracing of kernel events" OFF)
option(CMRX_CLANG_TIDY "Enable linting using Clang-tidy" ON)
option(CMRX_KERNEL_TRANSACTION_VERIFICATION "Enable checking for nested transaction commits" OFF)
option(CMRX_IDLE_THREAD_SHUTDOWN_CPU "Idle thread stops CPU" OFF)

option(CMRX_CUSTOM_FLASH_RANGE "If set to ON then developer SHALL provide custom range that covers all readable and executable area. If set to OFF then kernel will determine this automatically based on linker file, if possible" OFF)
set(CMRX_CUSTOM_FLASH_START 0 CACHE STRING "Custom start address of FLASH region. FLASH region includes this address")
set(CMRX_CUSTOM_FLASH_SIZE 0 CACHE STRING "Custom size of FLASH region")

option(CMRX_RPC_CANARY "Enable RPC canaries" OFF)
option(CMRX_MAP_FILE_WITH_EXTENSION "MAP file contains full name of binary with ELF extension (.elf.map)" OFF)
set(CMRX_CLANG_TIDY_LIBC_PATH /usr/include CACHE STRING "Path to standard C library used while linting")
set(OS_STACK_SIZE 1024 CACHE STRING "Stack allocated per thread in bytes")
set(OS_THREADS 8 CACHE STRING "Amount of entries in the thread table")
set(OS_PROCESSES 8 CACHE STRING "Amount of entries in the process table")
set(OS_STACKS 8 CACHE STRING "Amount of stacks allocated")

# List of CMake options that are transferred into nested build
set(CMRX_ALL_OPTIONS
    CMRX_ARCH_SMP_SUPPORTED
    CMRX_OS_NUM_CORES
    CMRX_KERNEL_TRACING
    CMRX_CUSTOM_FLASH_RANGE
    CMRX_CUSTOM_FLASH_START
    CMRX_CUSTOM_FLASH_SIZE
    OS_STACK_SIZE
    OS_THREADS
    OS_PROCESSES
    OS_STACKS
    )
