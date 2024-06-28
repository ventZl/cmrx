option(CMRX_ARCH_SMP_SUPPORTED "Architecture supports SMP and project is using it" OFF)
option(CMRX_OS_NUM_CORES "Amount of cores present in CPU package" INT:1)

set(CMRX_ALL_OPTIONS CMRX_ARCH_SMP_SUPPORTED CMRX_OS_NUM_CORES)
