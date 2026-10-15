Readme for Frama-C integration into CMRX
========================================

Frama-C formal verification tool support is integrated directly into CMRX buildsystem. Currently it allows to run analysis of portable portion of CMRX kernel. For this purpose a special HAL in `linux` platform is provided called `framac`. This HAL is stubbed, does not provide any real runtime support but provides stubs of abstract machine required by portable CMRX kernel.

If you want to perform CMRX kernel analysis, you need Frama-C installed. We recommend downloading Frama-C binary package, it already contains WP, RTE, Report and MdR plugins which this integration uses. `frama-c` binary must be in `PATH` variable.

To run analysis, create small stub CMake buildsystem which configures sources for static analysis and enables Frama-C support:

~~~~cmake
cmake_minimum_required(VERSION 3.22)

set(CMRX_ARCH linux)
set(CMRX_HAL frama-c)

include(cmrx/cmake/CMRX.cmake)

add_subdirectory(cmrx)
~~~~

This buildsystem does not define any executables, which is fine. The purpose of this build is to perform formal verification of code. Assuming that CMRX source code resides in `cmrx` subdirectory of `CMakeLists.txt` directory, run CMake like:

~~~~
cmake -B build .
cmake --build build - frama-c
~~~~

First line will configure the build for static analysis. This step will fail if Frama-C is not found in PATH. If it succeeds, it will enable generation of `compile_commands.json` and create target `frama-c` which runs formal verification.

Second line will run analysis itself. This analysis has plugins WP and WP-RTE enabled. It generates Markdown-formatted report, assuming `os_system_call` is code entrypoint. This report will be saved in `verification.md` file in `build` subdirectory. It also saves session for loading in Frama-C GUI. This session is saved in file `cmrx.framac` in `build` subdirectory.
