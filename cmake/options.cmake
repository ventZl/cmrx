option(CMRX_ARCH_SMP_SUPPORTED "Architecture supports SMP and project is using it" OFF)
option(CMRX_OS_NUM_CORES "Amount of cores present in CPU package" INT:1)
option(CMRX_UNIT_TESTS "Enable build of kernel unit tests" ON)

# List of CMake options that are transferred into nested build
set(CMRX_ALL_OPTIONS CMRX_ARCH_SMP_SUPPORTED CMRX_OS_NUM_CORES)
