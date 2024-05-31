C Microkernel Realtime eXecutive
================================

![unit tests](https://github.com/ventZl/cmrx/actions/workflows/unit_tests.yml/badge.svg)

This is realtime, microkernel operating system with fully static operation currently targeted towards Cortex-M microcontrollers having usable support for MPU. Currently microcontrollers based on Cortex-M0+, Cortex-M3, Cortex-M4 and Cortex-M7 are known to be supported.

Quickstart
----------

Add CMRX as a submodule of your repository:

~~~~~~
git submodule add https://github.com/ventZl/CMRX
~~~~~~

Assuming that the SDK for your microcontroller is based on CMSIS standard, you can use the following snippet in your top-level CMakeLists.txt to create firmware skeleton with one process:

~~~~~~
list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmrx/cmake)

# Set variables for FindCMSIS to detect and configure the CMSIS
set(CMSIS_ROOT path/to/cmsis/root/in/your/sdk)
set(DEVICE your_device_name)
set(CMSIS_LINKER_FILE path/to/your/linker/file)

include(FindCMSIS)

# Set variables for CMRX to select the right architecture and HAL
set(CMRX_ARCH arm)
set(CMRX_HAL cmsis)
# To get access to special commands
include(CMRX)

# To compile CMRX source
add_subdirectory(cmrx)

# Create firmware, main.c has to host void main(void) function
set(helloworld_SRCS main.c)
add_firmware(helloworld ${helloworld_SRCS})

# Create one process
set(init_SRCS init.c)
add_application(init ${init_SRCS})
target_link_libraries(init stdlib your_hal_lib)

# Link everything together
target_link_libraries(helloworld cmrx aux_systick your_hal_lib)
target_add_application(helloworld init)
~~~~~~

Depending on your HAL you might need to change this or add more declarations. 

Documentation
-------------

There is [documentation available online](https://ventZl.github.io/cmrx/). You can generate documentation using Doxygen directly from the source tree by running:

~~~~~
doxygen
~~~~~

in the root directory of this repository. HTML documentation will be generated into `doc/html` subdirectory.

Examples
--------
There are some examples of how to use CMRX with various HALs on various platforms in [Examples repository](https://github.com/ventZl/cmrx-examples).

Features
--------

 * microkernel fully isolated from the userspace
 * individual userspace processes fully isolated from each other
 * device drivers running in userspace with restricted access to peripheral memory area
 * priority-aware preemptive scheduler
 * basic POSIX-like API for managing threads, timers and signals
 * object-oriented remote procedure calling mechanism

