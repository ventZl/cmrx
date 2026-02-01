# Pico SDK RISC-V Quirk

This directory contains integration code for running CMRX on RP2350 RISC-V using Pico SDK.

## Problem

The CMRX RISC-V context switcher uses a "safe-point" model where pending context switches
are checked and executed at defined boundaries (typically trap/IRQ exit). The portable
RISC-V code provides the safe-point function but does not hook it into any specific
trap entry/exit mechanism.

On Pico SDK for RISC-V, the external IRQ handler (`isr_riscv_machine_external_irq`) is
declared weak in `crt0_riscv.S`, allowing it to be overridden.

## Solution

This quirk provides a replacement `isr_riscv_machine_external_irq` that:

1. Performs the same IRQ dispatch as the original Pico SDK handler
2. Calls `os_riscv_context_switch_safe_point()` after all IRQs are dispatched
3. Restores context and returns via `mret`

The safe-point call happens at a well-defined boundary where:
- All IRQs have been serviced
- IRQs are globally disabled (`mstatus.MIE` = 0)
- Caller-saved registers are saved on the stack

## Files

- `crt0_riscv_cmrx.S` - Replacement external IRQ handler with safe-point hook
- `CMakeLists.txt` - Build integration (activated when `PICO_PLATFORM=rp2350-riscv`)

## Usage

This quirk is automatically enabled when building CMRX for Pico SDK RISC-V. Ensure the
quirks subdirectory is included after the main CMRX target is defined.
