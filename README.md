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
 * (customizable) timing provider based on SysTick timer
 * userspace mutexes
 * notification API
 * queue library and queue server for cross-process queues
 * C11 clean
 * compatible with all SDKs providing CMSIS headers

Documentation
-------------

There is [documentation available online](https://ventZl.github.io/cmrx/). You can generate documentation using Doxygen directly from the source tree by running:

~~~~~
doxygen
~~~~~

in the root directory of this repository. HTML documentation will be generated into `doc/html` subdirectory.

Contributing
------------

CMRX is an open project and any contributions are welcome. You can help even if you don't feel like developing an operating system is your cup of coffee:

* provide feedback on your experience testing CMRX on real hardware - Have you tried to make CMRX run with your board? Let us know that if it works or not.
* reporting bugs - Did you find a bug? Did CMRX crash on you? Fill in a bug.
* suggesting enhancements - Do you think some feature is missing? Propose a feature.
* improve tests - Even though kernel does have some tests, the test suite can always be better.
* contribute code - Contributions in the form of pull requests that resolve issues, implement enhancements or feature requests are welcome as long as they
  adhere to the basic concept and mission of this project.

Reporting bugs
--------------

If you find a bug within the kernel itself, the best way to report the bug is to create a pull request containing a failing kernel test. We understand that this
is not always possible as fair amount of bugs will come from the integration and/or the build system, so the PR with failing test is not a must.

Examples
--------
There are some examples of how to use CMRX with various HALs on various platforms in [Examples repository](https://github.com/ventZl/cmrx-examples).
