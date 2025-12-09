Kernel unit testing
===================

CMRX kernel comes with its own unit test suite. This unit test suite tests soundness of kernel and some of library logic and can be run on developer's host machine. Unit test suite requires working GCC compiler for the host machine.

Philosophy
----------

CMRX is a naturally portable piece of code. Unit tests do perform white-box testing of the kernel code. Only portable code is tested as the goal of this level of tests is to test soundness of algorithms and data structures used in the kernel and selected libraries. These algorithms do the same thing regardless of the target architecture and if they are not correct while running unit tests they won't be correct when running on target hardware.

Unit tests aim ait testing 100% of functionality (specs of API docs) rather than 100% of code. Unit, for the purposes of unit tests is anything that bears an unit of functionality. It it not necessary to test each and every function individually. As the software under test is an embedded operating system, our unit tests do cheat a bit by inspecting internal state of the system here and there. Much of the information we need to observe in order to evaluate these tests is not visible from outside.

Enabling unit test suite
------------------------

Unit test suite is disabled by default. You can enable it in two ways:

* Your normal build - pass `CMRX_UNIT_TESTS=ON` argument to CMake. This will create nested binary directory inside your current binary directory, where the unit test build will be built. Once this is done, unit tests are built as a part of your normal build process. You can run unit tests using `unit_tests` target (e.g. `make unit_tests`).
* Dedicated binary directory - call CMake with argument `CMRX_UNIT_TESTS=ON` and use CMRX source directory as source directory for your project. This will create project that *only* builds and executes unit tests and nothing else. This will skip detection of your cross compiler, SDK and related stuff. Here you can run unit tests simply by using CMake standard `test` target (e.g. `make test`).

Test suite
----------

To speed things up the test suite is composed of just very small amount of test drivers. Currently there are only two drivers: one for kernel core and one for queue libraries. This saves time by not spinning tens of runners.

Each of these runners covers dozens of test cases.

Tests
-----

Tests for components are located in subdirectory called `tests` in directory where that component (e.g. kernel) is located. Thus, for example, kernel tests reside in src/os/kernel/tests subdirectory. All kernel tests are collected into one test driver. Failure of any test inside this driver will be reported as a failure of `test_kernel` test by CMake and CTest.

Writing tests
-------------

Kernel unit tests are using [ctest](https://github.com/bvdberg/ctest) framework. Follow the documentation of this framework to find out more on how it is used.

In general, tests are split into one file per API basis. So e.g. notification framework inside kernel is covered by `os_notify_wait_object.c` and this mostly covers all user-visible use-cases of this API. Tests are grouped into groups by subsystem they test. This is almost mapped 1:1 to source files for tests.

Code coverage
-------------

It is possible to generate code coverage report for unit test execution. GCC code coverage infrastructure is used for this purpose. If `gcov` is found in your system, then all unit tests are automatically instrumented to collect code coverage information. You can use target `coverage` (e.g. `make coverage`) to generate HTML code coverage report in the binary directory dedicated for unit test builds (see above where it lives depending on how you enabled unit testing).

Note that code coverage report is accumulative and if tests are executed multiple times without removing GCOV data files, results will contain sum of line execution of all test runs. This won't distort the information if line was executed or not but will definitely distort information on how many times line was actually executed.

