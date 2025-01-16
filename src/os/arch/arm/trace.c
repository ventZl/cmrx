#include <RTE_Components.h>
#include CMSIS_device_header

#undef COMP1
#undef COMP2

#include <cmrx/sys/trace.h>
#include <orbcode/trace/tpiu.h>
#include "swo.h"
#include <orbcode/trace/itm.h>
#include <orbcode/trace/dwt.h>

void trace_init_arm()
{
    SWOOptions swo = {
        .Protocol = SwoProtocolUart,
        .SwoPrescaler = 1111, /* 115200 kbaud @ 128 MHz*/
    };

    ITMOptions itm = {
        .TraceBusID = 0,
        .EnabledStimulusPorts = ITM_ENABLE_STIMULUS_PORTS_ALL,
        .EnableLocalTimestamp = 0,
        .EnableSyncPacket = 0,
        .ForwardDWT = 0,
        .GlobalTimestampFrequency = ITMGlobalTimestampFrequencyIfOutputFIFOEmpty,
        .LocalTimestampPrescaler = 0
    };
//    DWTOptions dwt;
    // TODO: setup DWT using by setting fields in dwt variable
    SWOSetup(&swo);
    // Apply configuration as defined in structs
    ITMSetup(&itm);
//    DWTSetup(&dwt);

    while (ITMIsPortEnabled(0)) {
        ITMWrite8(0, 'T');
        ITMWrite8(0, 'E');
        ITMWrite8(0, 'S');
        ITMWrite8(0, 'T');
        ITMWrite8(0, '\n');
    }
}

