@page getting_started Getting started

Here a step-by-step getting started guide is provided to start development using
CMRX as the RTOS. It covers all steps needed to start new project using
CMRX. In this guide, we will assume that the reader has prior experience with
developing basic bare metal applications in the embedded environment and is
aware of basic terms such as toolchain, linker scripts, HAL and binaries.

Obtain dependencies and CMRX
============================

As CMRX is based on CMake and is using Python internally, you need to get both
recent CMake and Python. The last tool, which is actually not a dependency, but
will make your life easier is Git. Now, you can establish your project by
creating an empty directory and initializing it as git repository, then you can
add CMRX as a sub-module in that repository:

~~~~~~~~~~~
mkdir project-dir
cd project-dir
git init
git submodule add https://github.com/ventZl/cmrx.git
git submodule update --init
~~~~~~~~~~~

This will create directory `cmrx` inside your project and retrieve current copy
of CMRX in there. This directory contains the source code, headers, CMake
modules and all the necessary auxiliary tools needed for CMRX to build.

Project template
================

Next, you can use following block as a template of your CMakeLists.txt

~~~~{.cmake}
cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmrx/cmake")

project(cmrx-example LANGUAGES C ASM)

# Tell CMRX what architecture and HAL is used
set(CMRX_ARCH arm)
set(CMRX_HAL cmsis)
include(CMRX)

add_subdirectory(cmrx)

# Define the main firmware entry with startup file
set(helloworld_SRCS main.c)
add_firmware(helloworld ${helloworld_SRCS})

# Define one userspace application executed by CMRX
set(hello_app_SRCS hello.c)
add_application(hello ${hello_app_SRCS})

target_link_libraries(helloworld cmrx)
target_add_applications(helloworld init)
~~~~

Above example makes a choice of the ARM target architecture and CMSIS-based HAL. More on
how to integrate with HALs below. It also mentions two source files: `main.c` and `hello.c`. Lets create both of them:

main.c:
~~~~~{.c}
#include <cmrx/cmrx.h>

int main(void)
{
    os_start();
}
~~~~~

hello.c
~~~~~{.c}
#include <cmrx/application.h>

static int main(void * unused)
{
    (void) unused;
    while (1) {
    }
    return 0;
}

CMRX_APPLICATION(init);
CMRX_APPLICATION_MMIO_RANGE(init);
CMRX_THREAD_CREATE(init, main, NULL, 64);
~~~~~

This constructs the minimal example. It is not buildable yet, because we have to integrate
startup files provided by the HAL somehow. CMRX using CMSIS expects that RTE_components.h
file is available in include path. This file shall be provided by the runtime, but usually
is not. There is a helper CMake module, which can try to find CMSIS components, set up
include paths, create RTE_Components.h file and create targets to build startup and system
components of HAL (if present).

To use this helper, you have to add lines similar to these before you include CMRX CMake
module:

~~~~~~~~
set(CMSIS_ROOT <path to CMSIS root directory>)
set(DEVICE <MCU name recognized by your CMSIS>)
set(CMSIS_LINKER_FILE <path to the linker file provided by the CMSIS>)
include(FindCMSIS)
~~~~~~~~

Actual usability of this helper depends on how much your HAL complies with CMSIS
specification. Some HALs are known to not supply linker scripts or to deviate in certain
details.

Then the project can be compiled using following commands:

    cmake -B build
    cmake --build build

This shall create a binary named `build/helloworld.elf`. You can load this file into your microcontroller and run it.
