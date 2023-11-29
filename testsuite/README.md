CMRX hardware automated testing suite
=====================================

The idea behind this test suite is to allow fast and seamless creation of tests which test CMRX core features.

Typical CMRX test is composed of a few parts:
* platform initialization - here only timing provider peripheral is important as CMRX itself does not care about any other peripheral
* CMRX kernel itself
* one or more applications running on top of CMRX optionally having threads and/or providing RPC services
* GDB script accompanying the software in case the test tests some specific behavior which can't be marked as passed or failed from the code itself
* openocd script to gain access to the target device

These parts can be divided into four groups:

1. Platform specific parts
--------------------------

This includes platform-specific initialization - generally the content of main.c, which initializes intended timing provider and openocd script
for connecting to target device. These mostly have nature of write once-per-platform and forget. Test suite will thus provide these for you
automatically if not provided by you.

Create files main.c and/or openocd.cfg if you wish to override the default.

2. CMRX-specific parts
----------------------

This mostly boils down to CMRX kernel and libraries. These are provided always and automatically linked to both core test driver and any possible 
applications requested by the test case.

3. Test-specific code 
---------------------

This is test, which carries your test case. It is specific for each and every test. In most cases, it contains one or more applications carrying some
code. You always have to supply this code. In simplest case you only have to provide the application code and you are good to go.

4. Test harness
---------------

This is a supplemental piece of GDB script and small piece of code linked to the test driver which aids the test to be executed and evaluated. The harness
is automatically linked to the binary and script is executed when the test driver is executed but for special scenarios both can be customized / extended.
See more details on test execution below

Automated test execution
========================

One test case transforms into one test driver. This is ineffective, because it creates a lot test drivers but there is no other way to reinitialize 
target hardware reliably.

Test is composed of three elements:
* test driver
* GDB script to connect to the target device, load, execute and evaluate the test 
* openocd configuration file which provides connection to the target device

Test is executed as follows:

GDB is launched provided by the script containing instructions for connecting to the target device. This script references openocd and openocd configuration
file. Another script is provided that contains definition of two breakpoints. One breakpoint is hit when function SUCCESS is called. It will cause GDB
to terminate with exit code 0. Another breakpoint is hit when function FAILURE is called. It will cause GDB to terminate with non-zero exit code.

Such behavior is generally recognized by most of the testing framework as fail / pass. It will also serve the needs of most test cases where test failure or 
success can be signalized by calling either function. In certain cases, one might need to trigger success or failure on different occasions. If this is 
needed then one can provide custom GDB script to be loaded and provide additional termination criteria.
