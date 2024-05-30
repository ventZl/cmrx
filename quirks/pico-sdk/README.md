Raspberry Pi Pico SDK quirk
===========================

The piece of code here is a replacement module which defines interrupt vectors.

This quirk is needed because the way how pico-sdk is organized causes, interrupt vectors can't be overriden by any software which resides in a library. CMRX kernel resides in a library, thus it is affected by this issue.

Thus, we do supply quick & dirty solution to this issue: modified version of crt0.S source from pico-sdk, which has ISRs our kernel is interested in claiming disabled. This solution is far from perfect and actually not sufficient in most applications where application itself wants to claim an ISR. Yet at least it allows us to boot on unmodified recent pico-sdk.
