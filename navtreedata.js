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
        [ "Prerequisites", "getting_started_cubemx.html#autotoc_md29", null ],
        [ "Creating project skeleton with CubeMX", "getting_started_cubemx.html#autotoc_md32", null ],
        [ "Basic CubeMX project settings", "getting_started_cubemx.html#autotoc_md34", null ],
        [ "Configuring the core", "getting_started_cubemx.html#autotoc_md36", null ],
        [ "Configuring GPIOs", "getting_started_cubemx.html#autotoc_md39", null ],
        [ "Generating project and getting CMRX source", "getting_started_cubemx.html#autotoc_md41", null ],
        [ "Integrating CMRX into CubeMX project", "getting_started_cubemx.html#autotoc_md43", [
          [ "1. Add CMake modules provided by CMRX into CMake module path", "getting_started_cubemx.html#autotoc_md45", null ],
          [ "2. Configure CubeMX integration", "getting_started_cubemx.html#autotoc_md47", null ],
          [ "3. Configure the CMRX kernel to use CubeMX", "getting_started_cubemx.html#autotoc_md49", null ]
        ] ],
        [ "Linking CMRX to the project", "getting_started_cubemx.html#autotoc_md51", null ],
        [ "Creating the blinky application", "getting_started_cubemx.html#autotoc_md53", null ],
        [ "CubeMX HAL tweaks", "getting_started_cubemx.html#autotoc_md56", null ],
        [ "Adding blinky application to the build", "getting_started_cubemx.html#autotoc_md59", null ],
        [ "Flashing the application", "getting_started_cubemx.html#autotoc_md61", null ]
      ] ],
      [ "HOWTO: Integrating CMRX into Pico-SDK project", "getting_started_picosdk.html", [
        [ "Prerequisites", "getting_started_picosdk.html#autotoc_md63", null ],
        [ "Creating Project Skeleton", "getting_started_picosdk.html#autotoc_md64", null ],
        [ "Integrating CMRX into Pico-SDK project", "getting_started_picosdk.html#autotoc_md65", [
          [ "1. Add CMake modules provided by CMRX and Pico-SDK into CMake module path", "getting_started_picosdk.html#autotoc_md66", null ],
          [ "2. Configuring Pico-SDK integration", "getting_started_picosdk.html#autotoc_md67", null ],
          [ "3. Configuring CMRX kernel to use Pico-SDK", "getting_started_picosdk.html#autotoc_md68", null ]
        ] ],
        [ "Linking CMRX to the project", "getting_started_picosdk.html#autotoc_md69", null ],
        [ "Creating main.c file", "getting_started_picosdk.html#autotoc_md70", null ],
        [ "Creating the blinky application", "getting_started_picosdk.html#autotoc_md71", null ],
        [ "Adding blinky application to the build", "getting_started_picosdk.html#autotoc_md72", null ],
        [ "Flashing the application", "getting_started_picosdk.html#autotoc_md73", null ]
      ] ]
    ] ],
    [ "Build system", "build_system.html", [
      [ "CMake support for CMRX", "build_system.html#autotoc_md0", null ],
      [ "Build-time options", "build_system.html#autotoc_md1", [
        [ "CMRX_UNIT_TESTS", "build_system.html#autotoc_md2", null ],
        [ "CMRX_KERNEL_TRACING", "build_system.html#autotoc_md3", null ],
        [ "CMRX_CLANG_TIDY", "build_system.html#autotoc_md4", null ],
        [ "CMRX_KERNEL_TRANSACTION_VERIFICATION", "build_system.html#autotoc_md5", null ],
        [ "CMRX_IDLE_THREAD_SHUTDOWN_CPU", "build_system.html#autotoc_md6", null ],
        [ "CMRX_RPC_CANARY", "build_system.html#autotoc_md7", null ],
        [ "CMRX_MAP_FILE_WITH_EXTENSION", "build_system.html#autotoc_md8", null ],
        [ "OS_STACK_SIZE", "build_system.html#autotoc_md9", null ],
        [ "OS_THREADS", "build_system.html#autotoc_md10", null ],
        [ "OS_PROCESSES", "build_system.html#autotoc_md11", null ],
        [ "OS_STACKS", "build_system.html#autotoc_md12", null ],
        [ "CMRX_ARCH_SMP_SUPPORTED", "build_system.html#autotoc_md13", null ],
        [ "CMRX_OS_NUM_CORES", "build_system.html#autotoc_md14", null ]
      ] ],
      [ "FindCMSIS CMake module", "findcmsis.html", null ],
      [ "CMRX CMake module", "cmrx-cmake.html", [
        [ "add_firmware()", "cmrx-cmake.html#autotoc_md22", null ],
        [ "add_application()", "cmrx-cmake.html#autotoc_md23", null ],
        [ "target_add_applications()", "cmrx-cmake.html#autotoc_md24", null ]
      ] ]
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
"group__arch__arm__syscall.html#ga0951838222a5c86294984e6b29ae87c9",
"group__os__timer.html#gaa0e811d6349480297cbad82c700077de"
];

var SYNCONMSG = 'click to disable panel synchronisation';
var SYNCOFFMSG = 'click to enable panel synchronisation';