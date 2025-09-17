Clang-tidy dummy project
========================

This is a dummy CMake project that creates no executable targets or libraries. Its only purpose is to configure CMRX kernel for some specific architecture so that clang-tidy can be ran on that build.

The reason why a build has to be configured for certain architecture is to find out which files actually belong to the build and which don't.

For this purpose a very barebone mock of CMSIS headers is provided in mock subdirectory that gets used by this CMakeLists.txt.

Using project
-------------

The use of this project is as follows:

cmake -B <binary_dir> -DCMRX_TARGET_PLATFORM=<platform>
cmake -B <binary_dir> clang-tidy

Aside from normal prerequisites to configure and build CMRX kernel, this project requires clang-tidy to be present in the system path.

The only target that is usable in this build is `clang-tidy` which executes clang-tidy on kernel sources.

Supported target platforms
--------------------------

As code paths may differ for different platforms, this project supports following target platforms for kernel configuration. They are supported by mocked CMSIS headers so that code is syntactically correct. In order to configure the build for target platform argument `-DCMRX_TARGET_PLATFORM` **must** be passed to CMake when configuring the build otherwise configuration will fail. Valid values of this parameter are provided in the following table with their explanation.

| CMRX_TARGET_PLATFORM | Platform properties |
|----------------------|---------------------|
| ARM_v6M              | ARM Cortex-M0+ CPU with MPU present |
| ARM_v7M              | ARM Cortex-M3 CPU with MPU present |
| ARM-v7MF             | ARM Cortex_M4 CPU with MPU and FPU both present |
