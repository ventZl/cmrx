SMP support readme
==================

CMRX was designed to support SMP. Support should be minimalistic, but working.
Unfortunately at the time of writing this support, the hardware debuggers and
their interface were in pretty bad shape for doing much work with multi-core
CPUs in SMP mode.

As of now the SMP support is known to be broken if launched on RP2040, the main
platform on which SMP support was developed (and pretty much the only reasonable
multi-core microcontroller than mortal human being can get by).

All effort was made to write the code in a way that it won't break running on
single-core system even if you enable the SMP support.

This may be revisited once more cooperative hardware will be available.
