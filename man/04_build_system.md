@page build_system Build system

CMRX is using CMake to build the kernel and applications. It works the best with vendor SDKs that also provide CMake-based build systems but can be integrated with SDKs that are based on Makefiles albeit this makes everything more complicated.

CMake support for CMRX
======================

CMRX support is currently provided as three more-less separate parts:

1. @subpage findcmsis which performs the detection and configuration of the CMSIS headers in vedor SDK.
2. @subpage cmrx-cmake that performe kernel configuration and provides some CMRX-specific commands.
3. Inclusion of CMRX source tree to actually build the CMRX kernel.




Build-time options
==================

CMRX kernel can be configured during the compile time. To do so, define any of the following variables before including CMRX.cmake module:

CMRX_UNIT_TESTS
---------------

Default: OFF

If enabled, CMRX kernel will automatically build unit test suite for the kernel. This requires the presence of GCC toolchain for the host machine.

CMRX_HIL_TESTS
--------------

Default: OFF

If enabled, calls of `add_hil_test` function from your CMakeLists.txt will add automatic tests using HIL tester infrastructure. With this option disabled, calls to `add_hil_tests` do nothing.

CMRX_INTEGRATION_TESTS
----------------------

Default: OFF

If enabled, will activate HIL tests (setting `CMRX_HIL_TESTS` to `ON`) and add set of kernel HW-SW integration tests.

CMRX_KERNEL_TRACING
-------------------

Default: OFF

If enabled, kernel tracing infrastructure is enabled. This will build callouts to tracing functions inside kernel. Tracing allows to trace certain events inside kernel into circular buffer that can be dumped via debugger and further analyzed.

CMRX_CLANG_TIDY
---------------

Default: ON

If enabled, clang-tidy target is added into build system. This target runs clang-tidy on kernel sources. Running this target requires clang-tidy to be installed on host machine. Build won't fail if this option is enabled and clang-tidy is not installed as this target is not run automatically.

CMRX_KERNEL_TRANSACTION_VERIFICATION
------------------------------------

Default: OFF

This enables additional runtime checks of kernel transaction subsystem. If rules of transaction handling are violated then kernel will assert. This option is for kernel debugging purposes only as it will slow kernel down significantly.

CMRX_IDLE_THREAD_SHUTDOWN_CPU
-----------------------------

Default: OFF

If enabled, then idle core will be halted using platform-specific way to stop the CPU. Doing so can help to save energy as core that is halted consumes almost no power. In rare cases this may cause the clock used to power the timing provider to stop too. In such cases this option has to be set to OFF or different clock source has to be used to power the timing provider.

CMRX_RPC_CANARY
---------------

Default: OFF

This enabled putting a canary value on stack during RPC processing. This allows early detection of stack corruption during RPC processing. Canaries are fixed values put onto stack when calling RPC method and checked for presence when RPC method is returning. Adding them slows execution of RPC calls a bit.

CMRX_MAP_FILE_WITH_EXTENSION
----------------------------

Default: OFF

If enabled, the MAP file name will contain both binary extension and ".map" suffix (forming firmware_name.elf.map). Otherwise the MAP file name only contains ".map" extension (forming firmware.map). This can be set to `ON` if vendor SDK forces the MAP file name to be ".elf.map" so all other tools will work with the correct file.

OS_STACK_SIZE
-------------

Default: 1024

Size of thread stack in bytes.

OS_THREADS
----------

Default: 8

Maximum amount of threads existing in system at any given time. No more threads can be created until some of threads quits.

OS_PROCESSES
------------

Default: 8

Maximum amount of processes existing in system at any given time. If more processes are added during CMake configuration run, then resulting firmware will fail to start.

OS_STACKS
---------

Default: 8

Maximum amount of stacks allocated in the system. Currently, it should never be smaller than OS_THREADS as this may result in threads not having stack to run.

CMRX_ARCH_SMP_SUPPORTED
-----------------------

Default: OFF

If enabled, the multi-core support is enabled for target platform. This enables per-core kernel startup and per-core thread allocation. **Experimental**

CMRX_OS_NUM_CORES
-----------------

Default: 1

Used to specify how many physical cores are there in package in case multi-core support has been enabled. This configures various in-kernel structures to have space for all required data. Note that it is possible to pass number lower than the actual core count if CMRX should not take care of all the cores in the system. For example, if the system contains four cores, this option may be configured to contain value 3. Then CMRX will manage only three cores, while the fourth core can be managed completely independently to CMRX kernel as long as it won't interfere with it.


@page findcmsis FindCMSIS CMake module

FindCMSIS is a CMake module provided by the CMRX source tree. Its main purpose is to find CMSIS header files, configure them and provide them as a linkable target. CMRX kernel configured to build against ARM CMSIS expects this target to exist.

Usage:
~~~~~~~~~~~~~~~
set(CMSIS_ROOT Path/To/CMSIS/headers)
set(DEVICE device_name)
set(DEVICE_INCLUDE_FILENAME device_header_filename)
set(CMSIS_LINKER_FILE Path/To/Linker/File)
set(SYSTEM_INCLUDE_FILENAME system_header_filename)

include(FindCMSIS)
~~~~~~~~~~~~~~~

This usage pattern assumes you added `cmake` subdirectory in CMRX source tree into `CMAKE_MODULE_PATH`. FindCMSIS will search the subtree of SDK for files that create the CMSIS headers. It will collect paths where these files are found and create target called `cmsis_headers` that exports these paths. It will also create `RTE_Components.h` file that can be included from any target linking `cmsis_headers` library. This way the platform header may be linked in a platform-independent way. CMRX kernel is using this mechanism for its CMSIS port supporting ARM Cortex-M platform. If any system or startup sources are found they are published in `CMSIS_SRCS` variable.

The use of configuration variables is as follows:

CMSIS_ROOT
----------

**Mandatory!** Provides the FindCMSIS with path where CMSIS headers are stored within vendor SDK. This cannot be detected automatically as all vendor SDKs use different paths and searching the whole SDK could slow the CMake down a lot.

DEVICE
------

**Mandatory!** Provides the device name recognized by the CMSIS headers in vendor SDK. This is used to configure CMSIS headers and create gate that can be used to include headers in a portable way. The name this variable is set to should be a device-specific define recognized by the system header file. It usually is the part name, but in certain rare cases it may be a wildcarded value. Consult the documentation of your SDK for valid values of this define.

CMSIS_LINKER_FILE
-----------------

**Mandatory!** Path to the linker file used to build the firmware. It must exist at the time of calling the CMake. CMRX kernel will analyze and patch it in order to support the automatic configuration of the memory protection unit.

DEVICE_INCLUDE_FILENAME
-----------------------

**Optional.** It can be used to explicitly specify the device include filename in case it does not follow the default pattern `${DEVICE}.h`. Can be omitted if there are no deviations and will be set to `${DEVICE}.h`.

SYSTEM_INCLUDE_FILENAME
-----------------------
**Optional.** This option can be used to explicitly specify the system include filename. It can be used in cases when the system include file doesn't follow the pattern `system_${DEVICE}.h`. This often happens in cases where SDK supports more than one microcontroller. May be omitted if the pattern is followed. In such cases the default pattern will be set as a value of this variable automatically.

@page cmrx-cmake CMRX CMake module

This module performs kernel configuration, generates runtime configuration header and includes platform-specific components.

Usage:
~~~~~~~~~~~~~~~~~~
set(CMRX_ARCH architecture)
set(CMRX_HAL hal)

include(CMRX)
~~~~~~~~~~~~~~~~~~

As of now the only supported target architecture is `arm` and the only supported HAL is `cmsis`.

CMRX_ARCH
---------

**Mandatory!** Specifies the architecture of the target microcontroller to be used. There must be architecture support layer provided for this architecture.

CMRX_HAL
--------

**Mandatory!** Specifies the HAL CMRX should use to access the target platform. One architecture may provide support for multiple HALs.

This module provides some commands that shall be used to automate actions that enable memory isolation support:

add_firmware()
==============

Usage:
~~~~~~~~~~~~~~~~
add_firmware(<binary_name> ...)
~~~~~~~~~~~~~~~~

This command is a wrapper around `add_executable()` CMake command. It works the same way. The only difference is that it automatically calls the GenLink script to generate binary-specific linker script. This is necessary to support memory protection.


add_application()
=================

Usage:
~~~~~~~~~~~~~~~~
add_application(<library_name> ...)
~~~~~~~~~~~~~~~~

This command is a wrapper around `add_library()` CMake command. When called, it will create a static library named `<library_name>` the same way as `add_library()` would do. Additionally it will call GenLink to create application-specific includes for the main include file. This dynamically adjusts application layout to support the memory protection.

target_add_applications()
=========================

Usage:
~~~~~~~~~~~~~~~~
target_add_applications(<binary_name> <library_name> ...)
~~~~~~~~~~~~~~~~

This command is a wrapper around `target_link_libraries()` CMake command. It will link the library containing application to the main executable. Aside from this it will modify the linker script of the firmware to include application instance into the build. Without this command being called the application won't be present in the firmware. Library linked this way must have been created using the `add_application()` command otherwise the call to `target_add_applications()` will fail.
