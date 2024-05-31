# The order of appearance here is important.
# While CMake tends to be declarative, in some cases the order of commands matters.
# You can freely link libraries which were not created yet, but you can't alter
# properties of libraries which were not created yet.
# Here the architecture-dependent portion of the kernel will agument the `os` target
# to provide low level functionality. Due to the circular dependencies between
# the platform-agnostic and architecture-dependent portion of kernel this is done
# by modifying the `os` target rather than defining own target.
add_subdirectory(kernel)
if (UNIT_TESTING)
    add_subdirectory(arch/testing)
else()
    add_subdirectory(arch/${CMRX_ARCH})
endif()


