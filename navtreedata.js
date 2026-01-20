/*
 @licstart  The following is the entire license notice for the JavaScript code in this file.

 The MIT License (MIT)

 Copyright (C) 1997-2020 by Dimitri van Heesch

 Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 and associated documentation files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge, publish, distribute,
 sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all copies or
 substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

 @licend  The above is the entire license notice for the JavaScript code in this file
*/
var NAVTREE =
[
  [ "C Microkernel Realtime eXecutive", "index.html", [
    [ "Introduction", "index.html", null ],
    [ "Static code analysis", "md_CLANG__TIDY.html", null ],
    [ "Hardware-in-the-loop testing", "md_HIL__TESTING.html", [
      [ "Creating tests", "md_HIL__TESTING.html#autotoc_md0", null ],
      [ "Test harness", "md_HIL__TESTING.html#autotoc_md1", null ],
      [ "Not using OpenOCD", "md_HIL__TESTING.html#autotoc_md2", null ],
      [ "Running tests", "md_HIL__TESTING.html#autotoc_md3", null ],
      [ "Kernel integration tests", "md_HIL__TESTING.html#autotoc_md4", null ]
    ] ],
    [ "Overview", "overview.html", [
      [ "Basic concepts", "concepts.html", null ],
      [ "Memory Model", "mem_model.html", null ],
      [ "Execution Model", "exec_model.html", null ],
      [ "Remote Procedure Calls", "rpc_intro.html", null ],
      [ "Transactional Processing", "txn_intro.html", null ],
      [ "Organization of the Code", "dev_code_organization.html", null ],
      [ "Development Environment", "dev_env.html", null ],
      [ "Integrating vendor SDKs", "hal_integration.html", null ],
      [ "SMP support", "smp_support.html", null ]
    ] ],
    [ "Getting started", "getting_started.html", [
      [ "HOWTO: Integrating CMRX into CubeMX project", "getting_started_cubemx.html", [
        [ "Prerequisites", "getting_started_cubemx.html#autotoc_md32", null ],
        [ "Creating project skeleton with CubeMX", "getting_started_cubemx.html#autotoc_md33", null ],
        [ "Basic CubeMX project settings", "getting_started_cubemx.html#autotoc_md34", null ],
        [ "Configuring the core", "getting_started_cubemx.html#autotoc_md35", null ],
        [ "Configuring GPIOs", "getting_started_cubemx.html#autotoc_md36", null ],
        [ "Generating project and getting CMRX source", "getting_started_cubemx.html#autotoc_md37", null ],
        [ "Integrating CMRX into CubeMX project", "getting_started_cubemx.html#autotoc_md38", [
          [ "1. Add CMake modules provided by CMRX into CMake module path", "getting_started_cubemx.html#autotoc_md40", null ],
          [ "2. Configure CubeMX integration", "getting_started_cubemx.html#autotoc_md41", null ],
          [ "3. Configure the CMRX kernel to use CubeMX", "getting_started_cubemx.html#autotoc_md44", null ]
        ] ],
        [ "Linking CMRX to the project", "getting_started_cubemx.html#autotoc_md47", null ],
        [ "Creating the blinky application", "getting_started_cubemx.html#autotoc_md48", null ],
        [ "CubeMX HAL tweaks", "getting_started_cubemx.html#autotoc_md50", null ],
        [ "Adding blinky application to the build", "getting_started_cubemx.html#autotoc_md52", null ],
        [ "Flashing the application", "getting_started_cubemx.html#autotoc_md54", null ]
      ] ],
      [ "HOWTO: Integrating CMRX into Pico-SDK project", "getting_started_picosdk.html", [
        [ "Prerequisites", "getting_started_picosdk.html#autotoc_md57", null ],
        [ "Creating Project Skeleton", "getting_started_picosdk.html#autotoc_md61", null ],
        [ "Integrating CMRX into Pico-SDK project", "getting_started_picosdk.html#autotoc_md63", [
          [ "1. Add CMake modules provided by CMRX and Pico-SDK into CMake module path", "getting_started_picosdk.html#autotoc_md64", null ],
          [ "2. Configuring Pico-SDK integration", "getting_started_picosdk.html#autotoc_md65", null ],
          [ "3. Configuring CMRX kernel to use Pico-SDK", "getting_started_picosdk.html#autotoc_md66", null ]
        ] ],
        [ "Linking CMRX to the project", "getting_started_picosdk.html#autotoc_md68", null ],
        [ "Creating main.c file", "getting_started_picosdk.html#autotoc_md70", null ],
        [ "Creating the blinky application", "getting_started_picosdk.html#autotoc_md71", null ],
        [ "Adding blinky application to the build", "getting_started_picosdk.html#autotoc_md72", null ],
        [ "Flashing the application", "getting_started_picosdk.html#autotoc_md73", null ]
      ] ]
    ] ],
    [ "Build system", "build_system.html", [
      [ "CMake support for CMRX", "build_system.html#autotoc_md5", null ],
      [ "Build-time options", "build_system.html#autotoc_md6", [
        [ "CMRX_UNIT_TESTS", "build_system.html#autotoc_md7", null ],
        [ "CMRX_HIL_TESTS", "build_system.html#autotoc_md8", null ],
        [ "CMRX_INTEGRATION_TESTS", "build_system.html#autotoc_md9", null ],
        [ "CMRX_KERNEL_TRACING", "build_system.html#autotoc_md10", null ],
        [ "CMRX_CLANG_TIDY", "build_system.html#autotoc_md11", null ],
        [ "CMRX_KERNEL_TRANSACTION_VERIFICATION", "build_system.html#autotoc_md12", null ],
        [ "CMRX_IDLE_THREAD_SHUTDOWN_CPU", "build_system.html#autotoc_md13", null ],
        [ "CMRX_RPC_CANARY", "build_system.html#autotoc_md14", null ],
        [ "CMRX_MAP_FILE_WITH_EXTENSION", "build_system.html#autotoc_md15", null ],
        [ "OS_STACK_SIZE", "build_system.html#autotoc_md16", null ],
        [ "OS_THREADS", "build_system.html#autotoc_md17", null ],
        [ "OS_PROCESSES", "build_system.html#autotoc_md18", null ],
        [ "OS_STACKS", "build_system.html#autotoc_md19", null ],
        [ "CMRX_ARCH_SMP_SUPPORTED", "build_system.html#autotoc_md20", null ],
        [ "CMRX_OS_NUM_CORES", "build_system.html#autotoc_md21", null ]
      ] ],
      [ "FindCMSIS CMake module", "findcmsis.html", null ],
      [ "CMRX CMake module", "cmrx-cmake.html", [
        [ "add_firmware()", "cmrx-cmake.html#autotoc_md29", null ],
        [ "add_application()", "cmrx-cmake.html#autotoc_md30", null ],
        [ "target_add_applications()", "cmrx-cmake.html#autotoc_md31", null ]
      ] ]
    ] ],
    [ "Kernel unit testing", "md_UNIT__TESTING.html", [
      [ "Philosophy", "md_UNIT__TESTING.html#autotoc_md81", null ],
      [ "Enabling unit test suite", "md_UNIT__TESTING.html#autotoc_md82", null ],
      [ "Test suite", "md_UNIT__TESTING.html#autotoc_md83", null ],
      [ "Tests", "md_UNIT__TESTING.html#autotoc_md84", null ],
      [ "Writing tests", "md_UNIT__TESTING.html#autotoc_md85", null ],
      [ "Code coverage", "md_UNIT__TESTING.html#autotoc_md86", null ]
    ] ],
    [ "Topics", "topics.html", "topics" ],
    [ "Data Structures", "annotated.html", [
      [ "Data Structures", "annotated.html", "annotated_dup" ],
      [ "Data Structure Index", "classes.html", null ],
      [ "Data Fields", "functions.html", [
        [ "All", "functions.html", null ],
        [ "Variables", "functions_vars.html", null ]
      ] ]
    ] ]
  ] ]
];

var NAVTREEINDEX =
[
"annotated.html",
"group__arch__arm__syscall.html#ga3e5ddb3df0d62f2dc357e64a3f04a6ce",
"group__os__syscall.html",
"structTimerEntry__t.html"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';