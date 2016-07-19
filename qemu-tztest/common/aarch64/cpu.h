#ifndef _CPU_H
#define _CPU_H

#define PSTATE_EL_EL0    0x00
#define PSTATE_EL_EL1H   0x04
#define PSTATE_EL_EL1T   0x05
#define PSTATE_EL_EL2H   0x08
#define PSTATE_EL_EL2T   0x09
#define PSTATE_EL_EL3H   0x0C
#define PSTATE_EL_EL3T   0x0D

#define SCR_NS      (1 << 0)
#define SCR_IRQ     (1 << 1)
#define SCR_FIQ     (1 << 2)
#define SCR_EA      (1 << 3)
#define SCR_NET     (1 << 6)
#define SCR_SMD     (1 << 7)
#define SCR_SCD     SCR_SMD    /* For AArch32 compatability */
#define SCR_HCE     (1 << 8)
#define SCR_SIF     (1 << 9)
#define SCR_RW      (1 << 10)
#define SCR_ST      (1 << 11)
#define SCR_TWI     (1 << 12)
#define SCR_TWE     (1 << 13)

#define CPACR_FPEN(_v) ((_v) << 20)

#define CPTR_TFP    (1 << 10)
#define CPTR_TCPAC  (1 << 31)

#define SCTLR_nTWI  (1 << 16)
#define SCTLR_nTWE  (1 << 18)

#define SPSR_EL0    PSTATE_EL_EL0
#define SPSR_EL1    PSTATE_EL_EL1T
#define SPSR_EL2    PSTATE_EL_EL2T
#define SPSR_EL3    PSTATE_EL_EL3T

#endif
