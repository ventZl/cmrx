# Pico SDK RISC-V Quirk

This directory contains integration code for running CMRX on RP2350 RISC-V using Pico SDK.

## Problem

The CMRX RISC-V context switcher uses a "safe-point" model where pending context switches
are checked and executed at defined boundaries (typically trap/IRQ exit). The portable
RISC-V code provides the safe-point function but does not hook it into any specific
trap entry/exit mechanism.

Pico SDK's `crt0_riscv.S` and `exception_table_riscv.S` declare the machine timer ISR
(`isr_riscv_machine_timer`) and the machine exception handler (`isr_riscv_machine_exception`)
as weak, allowing them to be overridden.

## Solution

This quirk provides full-`ExceptionFrame`-aware replacements for both weak handlers, plus
the ecall handler CMRX needs on top of them:

1. `isr_riscv_machine_timer` saves the complete `ExceptionFrame`, calls
   `cmrx_machine_timer_handler()` (the application-provided timing provider hook), then calls
   `os_riscv_context_switch_safe_point()` before restoring context and returning via `mret`.
2. `isr_riscv_machine_exception` saves the complete `ExceptionFrame`, dispatches to the
   individual exception handler for the trap cause (passing it the `ExceptionFrame` pointer),
   then also calls `os_riscv_context_switch_safe_point()` before restoring context and
   returning via `mret`.
3. `cmrx_ecall_handler.c` provides the `ecall` (syscall) exception handler invoked by (2):
   it advances `mepc` past the `ecall` instruction, dispatches the CMRX syscall, and writes
   the return value into the frame's `a0`.

In both (1) and (2), the safe-point call happens at a well-defined boundary where:
- The interrupt/exception has been fully serviced
- IRQs are globally disabled (`mstatus.MIE` = 0)
- The complete register state is saved on the stack (`ExceptionFrame`)

## Files

- `cmrx_timer_isr.c` - Machine timer ISR override with full `ExceptionFrame` save/restore.
- `cmrx_exception_dispatcher.c` - Machine exception handler override with full `ExceptionFrame`
  save/restore and exception table dispatch.
- `cmrx_ecall_handler.c` - `ecall` (syscall) exception handler for CMRX syscalls, invoked by
  the exception dispatcher above.
- `CMakeLists.txt` - Build integration.

## Usage

This quirk activates only when `PICO_PLATFORM` is set to `rp2350-riscv`; its `CMakeLists.txt`
returns immediately otherwise, so it is safe to always include it in a project even if that
project might also target other platforms. When active, it appends its sources to the `os`
target, adds CMRX's own `include/` directory as a private include path, and links
`pico_base_headers` and `hardware_regs`. Ensure the quirk's subdirectory is added after the
main CMRX target is defined, so it can attach its sources to `os`.
