Hardware-in-the-loop testing
============================

CMRX provides support for automated hardware-in-the-loop testing (HIL testing in the follwing text). The purpose of the HIL testing is to test your application functionality on your target hardware. CMRX build system provides some facilities to speed up and automate HIL testing as long as your test cases fit the framework CMRX provides.

The automation of HIL tests is built around GDB and OpenOCD. Currently this support also requires Linux (or compatible) as an execution platform. GDB is a mandatory tool but OpenOCD may be replaced by the GDB server of your choice if OpenOCD cannot provide connection to your target.

HIL tests work by creating a set of single-purpose binaries that are loaded and then executed one by one into your target hardware. Test harness expects that the code in question will reach either `TEST_SUCCESS` or `TEST_FAIL` function. Both these functions are intercepted by the debugger and when reached, GDB signals either test pass or fail depending on which of these functions was called. The implementation of these functions is provided in library called `hil_test_harness`. To use them, just link this library to your test binary.

The definition of test firmware is completely up to you and the test harness does not impose any additional limits other than your executable must end up calling one of `TEST_SUCCESS` or `TEST_FAIL` functions. If neither is called then the test is considered timed out after 10 seconds.

For more complicated tests there is support for sequencing using the function `TEST_STEP`. This function can be called with one argument, which denotes which test step was reached. First step expected to execute has number 1, all following steps have to be numbered in incrementing pattern (2, 3, 4). If `TEST_STEP` is called with argument out of sequence - smaller than previous or number not 1 larger than previous known step, the test will fail automatically.

Creating tests
--------------

An example `CMakeLists.txt` file to create a HIL test may look like:

~~~~~~~~~~~~~~~~~~~~
# Normal firmware definition, main.c contains main()
add_firmware(test_my_function src/main.c)
# Link HAL, timing provider (here SysTick-based) and standard library
target_link_libraries(test_my_function stm32cubemx aux_systick stdlib hil_test_harness)
# init, appl_1, svc_1 and drv_1 are names of applications added to the firmware
target_add_applications(test_my_function init appl_1 svc_1 drv_1)
# Use firmware test_my_function to perform HIL test visible in reports as "my_function"
add_hil_test(my_function test_my_function)
~~~~~~~~~~~~~~~~~~~~

This will define a binary named `test_my_function`. This is just a standard firmware for STM32 MCU based on CubeMX HAL. It contains several applications. Firmware itself links to the `hil_test_harness` and applications that want to call the `TEST_SUCCESS` or `TEST_FAIL` functions will probably have to do the same.

Then the command `add_hil_test` will register this firmware as a HIL test. This is just a wrapper around the `add_test` CMake command that calls GDB wrapper with some additional arguments.

Testing works by calling certain functions in the code of the test. There are three functions provided by the test harness existing in the test binaries:

* `TEST_SUCCESS` - called when test outcome is considered success
* `TEST_FAIL` - called when the test outcome is considered failure
* `TEST_STEP(step_number)` - called for each test step

These functions are just infinite loops, so test binary won't continue. Their sole purpose is to be attached by debugger and hook breakpoints on these functions. If the test execution breaks in the TEST_SUCCESS function, you know that it passed. If it breaks in the TEST_FAIL, you know it failed. Some tests also define sequence of events that have to happen using TEST_STEP. This function is called multiple times. The first call should happen with argument `step_number` set to 1 and every next call should be called with `step_number` incremented by one. If value of the argument in successive calls is out of order, then there is something wrong with the code and test should be considered failed.

These tests are running on the target hardware, using CMRX kernel configured using the same configuration as your firmware is using and calling same system calls your firmware is calling. So their execution is representative state of integration of CMRX on your platform.

The test harness hooks breakpoints on these fuctions automatically and can evaluate the pass/fail status of a test.

Test harness
------------

Test harness is split into two parts - the host side and firmware side.

The host side of the test harness is composed of GDB, OpenOCD, short stub that links the two together and some scripts to drive the harness. Almost all of this is fully automated, yet the integrator still has to provide two files:

* `test_init.gdb` - script that tells GDB how to connect to OpenOCD and load test into target
* `openocd.cfg` - script to configure OpenOCD to connect to your target.

Both these files should be stored in the top-level directory of your project, where the top-level `CMakeLists.txt` is.

`test_init.gdb` should contain instructions for GDB on how to connect to the OpenOCD (or any other GDB server) and how to load the firmware into target. Example `test_init.gdb` file may look like this:

~~~~~~~~~~~~~~~~~~
target extended-remote localhost:3333
load
~~~~~~~~~~~~~~~~~~

This code expects that GDB server knows how to load firmware into your microcontroller without any further instructions. If this is not the case then use the `monitor` GDB command to pass commands to GDB server to load the firmware.

`openocd.cfg` is a script that contains openocd commands used to connect openocd to your target. Don't put commands to load firmware in here as the OpenOCD instance is reused for multiple test runs. Test harness expects that once this script is done, OpenOCD is connected to the target and GDB can issue commands.

Not using OpenOCD
-----------------

If you don't want to use OpenOCD as your GDB server, you can set CMake option `CMRX_HIL_TESTING_SKIP_OPENOCD` to completely skip OpenOCD usage in HIL testing. If this option is set to ON, then test harness will completely ignore the `openocd.cfg` file presence and won't try to start OpenOCD for you. GDB server management is then integrator's responsibility. You still have to provide `test_init.gdb` file with valid instructions on how to connect to the GDB server and load the firmware.

Running tests
-------------

To activate test discovery, pass option `CMRX_HIL_TESTING` to CMake. This will enable actual test creation. Without this option set to true, commands to create tests do nothing. The goal of this semantics is to prevent littering of your CMake files by repetitive checks for HIL testing being enabled. Once this option is activated, your GDB initialization script, OpenOCD script and GDB executable will be evaluated. By default the GDB from system installation is used. If this is not suitable, you can pass CMake parameter `CMRX_GDB_PATH` pointing to GDB instance to be used for HIL testing.

All the tests added via `add_hil_test` will become a part of test suite of your project and they can be launched using `test` target (e.g. `make test` or `ninja test`).

Kernel integration tests
========================

This repository contains a set of kernel integration tests. These tests serve the purpose of automatically test that every supported feature of the kernel is working properly and as expected on your target hardware. This level of tests uses the HIL testing infrastructure internally. Integration tests work in tandem with unit tests, which test that the implementation of provided functionality is sound.

To activate integration tests, pass option `CMRX_INTEGRATION_TESTS` to CMake when configuring your build. As integration tests use HIL tester framework internally, this will automatically activate all HIL tests in your project as well.

Integration tests binaries are automatically generated from the content of `testsuite` subdirectory. The generator will automatically create binaries, applications and link CMRX kernel, and standard library. Each executable tests one specific requirement, such that as higher priority task will preempt lower priority task when resumed. This generates a lot of tests.

Integration testing subdirectory contains most of the content needed to build these tests except of the definition how to integrate these tests with your platform. It is up to you to provide two targets: `test_platform` and `test_platform_main` which integration tests can use to generate complete binaries.

Any CMRX firmware need to provide at least five basic components:
* Platform support code
* Main startup code
* Timing provider implementation
* CMRX kernel
* Application

In case of integration tests, the main startup code and CMRX kernel are provided and linked automatically by the test generator routine. The main startup code just initializes the timing provider and then launches the kernel. Applications are provided by tests themselves. Last two missing parts to be provided are timing provider and platform support code.

The way how to deliver these is by using `test_platform_main` CMake target.

Integration testing machinery expects that this target exists before CMRX has been included. It may be any kind of target that can be linked to the binary - e.g. static or interface library. It can provide public link libraries commands, source files to compile, compiler and linker switches to propagate to the code. Normally this is an interface library that links the HAL library or libraries and/or provides source files of HAL to be built. It should also choose timing provider implementation similar to one used in your firmware.

An example of `test_platform_main` for a project running on STM32CubeMX-generated platform and using SysTick-based timing provider:

~~~~~~~~~~~~~~~~~~~~
add_library(test_platform_main INTERFACE)
target_link_libraries(test_platform_main PUBLIC stm32cubemx aux_systick)
target_compile_options(test_platform_main INTERFACE -Werr)
~~~~~~~~~~~~~~~~~~~~

The `stm32cubemx` target is one created by the CubeMX package residing in one of generated CMake files. This target contains all the source files and headers of HAL and system startup code. It also injects `-Werr` compiler switch into every test so that each warning in the test build will cause build to fail. When integration test binaries are created, the `test_platform_main` is linked to every executable created, affecting compile options and injecting libraries.

It is possible that some of tests may need some platform-specific code. For such cases there is another target called `test_platform` which is linked to every application created in every test. Here you can specify dependencies needed for tests in order to compile. Normally, this target may be left empty but sometimes it is necessary to inject platform headers into all compilation units, which can be done using this target. Similar to above target, this has to be target usable as library linked to the application carrying test. If nothing specific is needed, you can pass it as empty interface library like:

~~~~~~~~~~~~~~~~~~~~
# This target is intentionally empty
add_library(test_platform INTERFACE)
~~~~~~~~~~~~~~~~~~~~

Once these two targets exist and provide instructions on how to create working binaries for your target using your HAL and startup code, you can enable integration testing.
