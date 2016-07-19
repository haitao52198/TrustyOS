#ifndef _EXCEPTION_H
#define _EXCEPTION_H

#define EC_UNKNOWN 0x00
#define EC_WFI_WFE 0x01
#define EC_SIMD 0x07
#define EC_ILLEGAL_STATE 0x0E
#define EC_SVC32 0x11
#define EC_SMC32 0x13
#define EC_SVC64 0x15
#define EC_SMC64 0x17
#define EC_SYSINSN 0x18
#define EC_IABORT_LOWER 0x20
#define EC_IABORT 0x21
#define EC_DABORT_LOWER 0x24
#define EC_DABORT 0x25

#ifndef __ASSEMBLY__
typedef union {
    struct {
        uint32_t dfsc : 6;
        uint32_t wnr : 1;
        uint32_t s1ptw : 1;
        uint32_t cm : 1;
        uint32_t ea : 1;
        uint32_t fnv  : 1;
        uint32_t res0 : 3;
        uint32_t ar : 1;
        uint32_t sf : 1;
        uint32_t srt : 5;
        uint32_t sse: 1;
        uint32_t sas: 2;
        uint32_t isv: 1;
    };
    uint32_t raw;
} armv8_data_abort_iss_t;

typedef union {
    struct {
        uint32_t ifsc : 6;
        uint32_t res0_0 : 1;
        uint32_t s1ptw : 1;
        uint32_t res0_1 : 1;
        uint32_t ea : 1;
        uint32_t fnv  : 1;
        uint32_t res0_2 : 3;
    };
    uint32_t raw;
} armv8_inst_abort_iss_t;
#endif

#endif
