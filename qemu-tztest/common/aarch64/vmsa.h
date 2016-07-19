#ifndef _ARMV8_VMSA_H
#define _ARMV8_VMSA_H

/*
typedef union {
    struct {
        uint64_t type : 2;
        uint64_t attridx : 3;
        uint64_t ns : 1;
        uint64_t ap2_1 : 2;
        uint64_t sh1_0 : 2;
        uint64_t af : 1;
        uint64_t ng : 1;
        uint64_t pa : 36;
        uint64_t res0 : 4;
        uint64_t contig : 1;
        uint64_t pxn : 1;
        uint64_t xn : 1;
    };
    uint64_t raw;
} armv8_4k_pg_pte_t;

typedef union {
    struct {
        uint64_t type : 2;
        uint64_t ignored0 : 10;
        uint64_t pa : 36;
        uint64_t res0 : 4;
        uint64_t ignored1 : 7;
        uint64_t pxn : 1;
        uint64_t xn : 1;
        uint64_t ap : 2;
        uint64_t ns : 1;
    };
    uint64_t raw;
} armv8_4k_tbl_pte_t;
*/

#define PAGE_SIZE 0x1000
#define ARMV8_PAGE_ATTRINDX_SHIFT 2
#define ARMV8_PAGE_NS_SHIFT 5
#define ARMV8_PAGE_AP_SHIFT 6
#define ARMV8_PAGE_SH_SHIFT 8
#define ARMV8_PAGE_AF_SHIFT 10
#define ARMV8_PAGE_NG_SHIFT 11
#define ARMV8_PAGE_CONIG_SHIFT 52
#define ARMV8_PAGE_PXN_SHIFT 53
#define ARMV8_PAGE_XN_SHIFT 54

#define ARMV8_PAGE_EL1_RW (0<<ARMV8_PAGE_AP_SHIFT)
#define ARMV8_PAGE_EL0_RW (1<<ARMV8_PAGE_AP_SHIFT)
#define ARMV8_PAGE_EL1_R  (2<<ARMV8_PAGE_AP_SHIFT)
#define ARMV8_PAGE_EL0_R  (3<<ARMV8_PAGE_AP_SHIFT)
#define ARMv8_PAGE_ACCESS (1<<ARMV8_PAGE_AF_SHIFT)

#define PTE_ACCESS      ARMv8_PAGE_ACCESS
#define PTE_PRIV_RW     ARMV8_PAGE_EL1_RW
#define PTE_PRIV_RO     ARMV8_PAGE_EL1_R
#define PTE_USER_RW     ARMV8_PAGE_EL0_RW
#define PTE_USER_RO     ARMV8_PAGE_EL0_R
#define PTE_PAGE 0x3
#define PTE_TABLE 0x3
#endif
