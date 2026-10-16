# The order of appearance here is important.
# While CMake tends to be declarative, in some cases the order of commands matters.
# You can freely link libraries which were not created yet, but you can't alter
# properties of libraries which were not created yet.
# Here the architecture-dependent portion of the kernel will agument the `os` target
# to provide low level functionality. Due to the circular dependencies between
# the platform-agnostic and architecture-dependent portion of kernel this is done
# by modifying the `os` target rather than defining own target.

add_subdirectory(kernel)
if (UNIT_TESTING_BUILD)
    add_subdirectory(arch/testing)
    # Testing pseudo-architecture is considered generic
    target_compile_definitions(os PRIVATE -DCMRX_PLATFORM_GENERIC)
else()
    add_subdirectory(arch/${CMRX_ARCH})
    if (CMRX_PLATFORM)
        if (EXISTS arch/${CMRX_ARCH}/${CMRX_PLATFORM})
            add_subdirectory(arch/${CMRX_ARCH}/${CMRX_PLATFORM})
        endif()
        # Platform directory doesn't exist, assume generic platform
        target_compile_definitions(os PRIVATE -DCMRX_PLATFORM_GENERIC)
    else()
        # No platform specified
        target_compile_definitions(os PRIVATE -DCMRX_PLATFORM_GENERIC)
    endif()

endif()


