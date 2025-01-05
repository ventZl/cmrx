C Microkernel Realtime eXecutive
================================

![unit tests](https://github.com/ventZl/cmrx/actions/workflows/unit_tests.yml/badge.svg)

CMRX is realtime microkernel operating system with fully static operation targeted towards Cortex-M microcontrollers having usable support for MPU. Currently microcontrollers based on Cortex-M0+, Cortex-M3, Cortex-M4 and Cortex-M7 are known to be supported.

Quickstart
----------

See [step-by-step guide](man/03_getting_started.md) on how to integrate CMRX into your project.

Features
--------

 * microkernel fully isolated from the userspace
 * individual userspace processes fully isolated from each other
 * automatic memory protection management
 * device drivers running in userspace with restricted access to peripheral memory area
 * priority-aware preemptive scheduler
 * basic POSIX-like API for managing threads, timers and signals
 * object-oriented remote procedure calling mechanism

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

