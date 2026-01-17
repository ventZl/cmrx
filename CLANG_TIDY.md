Static code analysis
====================

CMRX integrates support for Clang-tidy static code analyzer. Unlike unit tests, which only test the portable part of the code clang-tidy checks all the code for given platform. For this purpose there is a special project inside [testing/clang-tidy] subdirectory. This project is a standalone empty project that integrates CMRX kernel and abstract machine implementation for given target platform which is then subject of code analysis.

If you want to run static clang-tidy analysis of CMRX codebase, then do the following:

~~~~~~~~~~~~~~~~~~~~~~~~~
cmake -DCMRX_TARGET_PLATFORM=<platform> -B clang-tidy-build $CMRX_ROOT/testing/clang-tidy
cmake --build clang-tidy-build -- clang-tidy
~~~~~~~~~~~~~~~~~~~~~~~~~

This requires clang compiler to be present on your machine. The `CMRX_TARGET_PLATFORM` argument may have one of the following values:

| CMRX_TARGET_PLATFORM | Target platform |
|----------------------|-----------------|
| ARMv6M | ARM Cortex-M0+ MCUs |
| ARMv7M | ARM Cortex-M3, M4, M7 MCUs without FPU active |
| ARMv7MF | ARM Cortex-M4 and M7 MCUs with FPU activated |
| ARMv8M-Baseline | ARM Cortex-M23 MCUs (ARMv8M-Baseline architecture) |
| ARMv8M-Mainline | ARM Cortex-M33, M35P, M55, M85 MCUs (ARMv8M-Mainline architecture) |

As each of these platforms is using different code path, you have to select which platform you want to activate before running static code analysis. The project for running analyses is not depending on any external SDK or CMSIS pack. It provides very minimal mock of CMSIS headers which is required to perform the analysis. This project does not even define any firmware to be built so `make all` does nothing.

