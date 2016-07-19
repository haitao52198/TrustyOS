#ifndef _CPU_H
#define _CPU_H

#define CPSR_M_USR  0x10
#define CPSR_M_FIQ  0x11
#define CPSR_M_IRQ  0x12
#define CPSR_M_SVC  0x13
#define CPSR_M_MON  0x16
#define CPSR_M_ABT  0x17
#define CPSR_M_HYP  0x1A
#define CPSR_M_UND  0x1B
#define CPSR_M_SYS  0x1F

#define SCR_NS      (1 << 0)
#define SCR_IRQ     (1 << 1)
#define SCR_FIQ     (1 << 2)
#define SCR_EA      (1 << 3)
#define SCR_FW      (1 << 4)
#define SCR_AW      (1 << 5)
#define SCR_NET     (1 << 6)
#define SCR_SCD     (1 << 7)
#define SCR_SMD     SCR_SCD    /* For AArch64 compatability */
#define SCR_HCE     (1 << 8)
#define SCR_SIF     (1 << 9)
#define SCR_TWI     (1 << 12)
#define SCR_TWE     (1 << 13)

#define CPACR_FPEN(_v) ((_v) << 20)

#define CPTR_TFP    (1 << 10)
#define CPTR_TCPAC  (1 << 31)

#define NSACR_CP10  (1 << 10)

#define SCTLR_nTWI  (1 << 16)
#define SCTLR_nTWE  (1 << 18)

#define CPSR_F      (1 << 6)
#define CPSR_I      (1 << 7)
#define CPSR_A      (1 << 8)

#define SPSR_EL0    CPSR_M_USR
#define SPSR_EL1    CPSR_M_SVC
#define SPSR_EL2    CPSR_M_HYP
#define SPSR_EL3    CPSR_M_MON

#endif
