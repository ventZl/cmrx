<p align="center">
  <img alt="cmrx logo" src="man/logo.png">
</p>

CMRX - Microkernel RTOS for microcontrollers
============================================

![unit tests](https://github.com/ventZl/cmrx/actions/workflows/unit_tests.yml/badge.svg) ![stm32h7 build](https://github.com/ventZl/cmrx/actions/workflows/stm32h7-build.yml/badge.svg) ![stm32g4 build](https://github.com/ventZl/cmrx/actions/workflows/stm32g4-build.yml/badge.svg) ![rp2040 build](https://github.com/ventZl/cmrx/actions/workflows/rp2040-build.yml/badge.svg)

**Table of Contents**
- [About](#about)
- [Quickstart](#quickstart)
- [Features](#features)
- [Documentation](#documentation)
- [Using CMRX](#using-cmrx)
- [Contributing](#contributing)
- [Reporting bugs](#reporting-bugs)
- [Examples](#examples)

# About

CMRX is a microkernel-based realtime operating system providing full memory isolation of userspace running on low-power microcontrollers.

Main goal of this project is to create runtime environment with full memory isolation as a non-negotiable feature that is always active while providing all the features users are used to have in RTOS systems without things breaking randomly.

We want remove the opportunity to say "it is complicated" as an excuse to avoid memory isolation in embedded systems.

CMRX RTOS supports fully static operation with compile-time configuration of all important aspects.

The footprint of the CMRX RTOS is small. Fully featured kernel occupies roughly 13 kB of FLASH and needs around 2kB of RAM. RAM consumption depends on compile-time configuration, especially amount of threads and processes configured.

CMRX RTOS supports Cortex-M microcontrollers with Cortex-M MPU peripheral present. Microcontrollers based on Cortex-M0+, Cortex-M3, Cortex-M4 and Cortex-M7 cores are supported.

# Quickstart

See [step-by-step guide](https://ventzl.github.io/cmrx/getting_started.html) on how to integrate CMRX into your project.

# Features

 * microkernel design to minimize code running in privileged mode
 * lightweight
 * Cortex-M0+, Cortex-M3, Cortex-M4, Cortex-M7 MCUs supported
 * support for floating-point hardware
 * compile-time configurable
 * kernel fully isolated from the userspace
 * individual userspace processes fully isolated from each other
 * automatic memory protection management - no manual zone/partition configuration
 * device drivers running in userspace with restricted access to peripheral memory area
 * priority-aware preemptive scheduler
 * basic POSIX-like API for managing threads, timers and signals
 * object-oriented remote procedure calling mechanism
 * (customizable) timing provider based on SysTick timer
 * userspace mutexes
 * notification API
 * queues API
 * C11 clean
 * compatible with all vendor SDKs providing CMSIS headers
 
# Documentation

There is [documentation available online](https://ventZl.github.io/cmrx/). You can generate documentation using Doxygen directly from the source tree by running:

~~~~~
doxygen
~~~~~

in the root directory of this repository. HTML documentation will be generated into `doc/html` subdirectory. This requires Doxygen to be installed on your machine.

# Using CMRX

CMRX is an operating system designed to be used with vendor-provided SDKs. CMRX does not provide explicit support for any board or target. Rather it provides a way how to hook itself up to a vendor SDK. This way the use of the CMRX is not limited by the list of supported hardware. All hardware providing CMSIS headers in its SDK is supported out of box (virtually all current SDKs do provide CMSIS headers).

With security in mind, CMRX employs memory isolation as a non-negotiable feature. The aim is at making this feature usable without the need of extensive learning how it works. To make this happen CMRX retrofits the concept of process into RTOS world.

All the user code has to be organized into processes. While this may sound complicated in fact it is quite easy: just put your code into libraries. One library = one process. Threads defined inside process can access all the data residing in the same process. No other data can be accessed by the code inside process.

To break out of the container created by the process the remote procedure calling service is provided by the kernel. This allows connecting code living in various processes without compromising the memory protection mechanism and without the developer having to understand how memory isolation works.

CMRX is using features of the CMake build system. This allows certain actions to be taken fully automatically guaranteeing that the state of the build is fully consistent.

# Contributing

CMRX is an open project and any external contributions are welcome. You can help even if you don't feel like developing an operating system is your cup of coffee:

* provide feedback on your experience testing CMRX on real hardware - Have you tried to make CMRX run with your board? Let us know that if it works or not.
* improve the documentation - Is the documentation missing something? Help to improve it!
* reporting bugs - Did you find a bug? Did CMRX crash on you? Fill in a bug.
* suggesting enhancements - Do you think some feature is missing? Propose a feature.
* improve tests - Even though kernel does have some tests, the test suite can always be better.
* contribute code - Contributions in the form of pull requests that resolve issues, implement enhancements or feature requests are welcome as long as they adhere to the basic concept and mission of this project.

# Reporting bugs

If you find a bug within the kernel itself, the best way to report the bug is to create a pull request containing a failing kernel test.

We understand that this is not always possible as fair amount of bugs will come from the integration and/or the build system, so the PR with failing test is not a must.

Use the issues section here on GitHub to report bugs.

# Examples

There are some examples of how to use CMRX with various HALs on various platforms in [Examples repository](https://github.com/ventZl/cmrx-examples).
