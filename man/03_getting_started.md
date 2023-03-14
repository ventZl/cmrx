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

~~~~
cmake_minimum_required(VERSION 3.12)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmrx/cmake")

project(cmrx-example LANGUAGES C ASM)

include(CMRX)

set(helloworld_SRCS main.c)
add_firmware(helloworld ${helloworld_SRCS})

set(hello_app_SRCS hello.c)
add_application(hello ${hello_app_SRCS})

target_link_libraries(helloworld init cmrx)
~~~~

This `CMakeLists.txt` mentions two source files: `main.c` and `hello.c`. Lets create 
both of them:

main.c:
~~~~~{.c}
#include <cmrx/os/sched.h>

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

This constructs the minimal buildable example. It can be compiled using following commands:

    cmake -B build
    cmake --build build

This shall create a binary named `build/helloworld.elf`. You can load this file into your microcontroller and run it.
