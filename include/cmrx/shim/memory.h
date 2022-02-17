#pragma once

#warning "These values are hardcoded and most probably incorrect"

#if (defined NRF52)

#define code_base()				(0)

#else

#define code_base()				(0x10000000)

#endif

#define code_size()				(2*1024*1024)
