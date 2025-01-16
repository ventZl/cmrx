#pragma once
#include <stddef.h>
#include <cmrx/assert.h>

typedef struct {
    uint32_t RESERVED[4];
    __IOM uint32_t CODR;
    uint32_t RESERVED2[55];
    __IOM uint32_t SPPR;
    uint32_t RESERVED3[131];
    __IM uint32_t FFSR;
    uint32_t RESERVED4[807];
    __IOM uint32_t CLAIMSET;
    __IOM uint32_t CLAIMCLR;
    uint32_t RESERVED5[2];
    __OM uint32_t LAR;
    __IM uint32_t LSR;
    __IOM uint32_t AUTHSTAT;
    uint32_t RESDERVED6[2];
    __IM uint32_t DEVID;
    __IM uint32_t DEVTYPE;
    __IM uint32_t PIDR4;
    uint32_t RESDERVED7[4];
    __IM uint32_t PIDR0;
    __IM uint32_t PIDR1;
    __IM uint32_t PIDR2;
    __IM uint32_t PIDR3;
    __IM uint32_t CIDR0;
    __IM uint32_t CIDR1;
    __IM uint32_t CIDR2;
    __IM uint32_t CIDR3;

} SWO_Type;

#define CODR_PRESCALER_Pos 0
#define CODR_PRESCALER_Msk (0x1FFFUL /*<< CODR_PRESCALER_Pos */)

#define SPPR_PPROT_Pos 0
#define SPPR_PPROT_Msk (0x3UL << SPPR_PPROT_Pos)

#define SPPR_PPROT_MANCHESTER   1
#define SPPR_PPROT_UART         2

#define FFSR_FTNONSTOP_Pos 3
#define FFSR_TCPRESENT_Pos 2
#define FFSR_FTSTOPPED_Pos 1
#define FFSR_FLINPROG_Pos 0

#define FFSR_FTNONSTOP_Msk (1 << FFSR_FTNONSTOP_Pos)
#define FFSR_TCPRESENT_Msk (1 << FFSR_TCPRESENT_Pos)
#define FFSR_FTSTOPPED_Msk (1 << FFSR_FTSTOPPED_Pos)
#define FFSR_FLINPROG_Msk (1 << FFSR_FLINPROG_Pos)

#define CLAIMSET_CLAIMSET_Pos 0
#define CLAIMSET_CLAIMSET_Msk (0xFUL << CLAIMSET_CLAIMSET_Pos)

#define CLAIMCLR_CLAIMCLR_Pos 0
#define CLAIMCLR_CLAIMCLR_Msk (0xFUL << CLAIMCLR_CLAIMCLR_Pos)

#define LAR_ACCESS_W_Pos 0
#define LAR_ACCESS_W_Msk (0xFFFFFFFFUL << LAR_ACCESS_W_Pos)

#define LSR_LOCKTYPE_Pos 2
#define LSR_LOCKTYPE_Msk (1 << LSR_LOCKTYPE_Pos)

#define LSR_LOCKGRANT_Pos 1
#define LSR_LOCKGRANT_Msk (1 << LSR_LOCKGRANT_Pos)

#define LSR_LOCKEXIST_Pos 0
#define LSR_LOCKEXIST_Msk (1 << LSR_LOCKEXIST_Pos)

#define SWO_SWTF_ACCESS_W   0xC5ACCE55UL

#define SWO_BASE            0x5C003000UL

#define SWO ((SWO_Type *) SWO_BASE)

_Static_assert(offsetof(SWO_Type, CODR) == 0x10);
_Static_assert(offsetof(SWO_Type, SPPR) == 0xF0);
_Static_assert(offsetof(SWO_Type, FFSR) == 0x300);
_Static_assert(offsetof(SWO_Type, CLAIMSET) == 0xFA0);
_Static_assert(offsetof(SWO_Type, LAR) == 0xFB0);
_Static_assert(offsetof(SWO_Type, DEVID) == 0xFC4);
_Static_assert(offsetof(SWO_Type, PIDR0) == 0xFE0);

typedef struct {
    uint32_t CTRL;
    uint32_t PRIORITY;
    uint32_t RESERVED[998];
    __IOM uint32_t CLAIMSET;
    __IOM uint32_t CLAIMCLR;
    uint32_t RESERVED2[2];
    __OM uint32_t LAR;
    __IM uint32_t LSR;
    __IOM uint32_t AUTHSTAT;
    uint32_t RESDERVED3[2];
    __IM uint32_t DEVID;
    __IM uint32_t DEVTYPE;
    __IM uint32_t PIDR4;
    uint32_t RESDERVED4[4];
    __IM uint32_t PIDR0;
    __IM uint32_t PIDR1;
    __IM uint32_t PIDR2;
    __IM uint32_t PIDR3;
    __IM uint32_t CIDR0;
    __IM uint32_t CIDR1;
    __IM uint32_t CIDR2;
    __IM uint32_t CIDR3;
} SWTF_Type;

#define SWTF_BASE            0x5C004000UL

#define SWTF ((SWTF_Type *) SWTF_BASE)

_Static_assert(offsetof(SWTF_Type, CLAIMSET) == 0xFA0);
_Static_assert(offsetof(SWTF_Type, LAR) == 0xFB0);
_Static_assert(offsetof(SWTF_Type, DEVID) == 0xFC4);
_Static_assert(offsetof(SWTF_Type, PIDR0) == 0xFE0);

#define CTRL_ENS0_Pos 0
#define CTRL_ENS0_Msk (1 << CTRL_ENS0_Pos)

typedef enum {
    SwoProtocolManchester = 1,
    SwoProtocolUart = 2
} SwoProtocol;

typedef struct {
    SwoProtocol Protocol;
    int SwoPrescaler;
} SWOOptions;

bool SWOSetup(const SWOOptions * options)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DBGMCU->CR = DBGMCU_CR_DBG_TRACECKEN | DBGMCU_CR_DBG_CKD3EN | DBGMCU_CR_DBG_CKD1EN;

    SWO->LAR = SWO_SWTF_ACCESS_W;
    ASSERT((SWO->LSR & LSR_LOCKGRANT_Msk) == 0);

    SWTF->LAR = SWO_SWTF_ACCESS_W;
    ASSERT((SWO->LSR & LSR_LOCKGRANT_Msk) == 0);

    SWTF->CTRL = CTRL_ENS0_Msk;

    switch (options->Protocol) {
        case SwoProtocolManchester:
            SWO->SPPR = SPPR_PPROT_MANCHESTER << SPPR_PPROT_Pos;
            break;

        case SwoProtocolUart:
            SWO->SPPR = SPPR_PPROT_UART << SPPR_PPROT_Pos;
            break;

        default:
            ASSERT(0);
            return false;
    }

    SWO->CODR = options->SwoPrescaler - 1;

    return true;
}
