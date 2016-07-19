#define __ASSEMBLY__
#include "smc.h"
#include "cpu.h"
#undef __ASSEMBLY__

.section .text
.macro SAVE_SYS_PAIR reg1, reg2, idx
    mrs x1, \reg1
    mrs x2, \reg2
    stp x1, x2, [x0, #\idx]
.endm

.macro RESTORE_SYS_PAIR reg1, reg2, idx
    ldp x1, x2, [x0, #\idx]
    msr \reg1, x1
    msr \reg2, x2
.endm

.macro MONITOR_SAVE_STATE
    SAVE_SYS_PAIR elr_el3, spsr_el3, 0*16
    SAVE_SYS_PAIR elr_el1, spsr_el1, 1*16
    SAVE_SYS_PAIR esr_el1, sp_el1, 2*16
    SAVE_SYS_PAIR spsel, sp_el0, 3*16
//    SAVE_SYS_PAIR spsr_abt, spsr_und, 4*16
//    SAVE_SYS_PAIR spsr_irq, spsr_fiq, 5*16
    SAVE_SYS_PAIR sctlr_el1, actlr_el1, 6*16
    SAVE_SYS_PAIR cpacr_el1, csselr_el1, 7*16
    SAVE_SYS_PAIR ttbr0_el1, ttbr1_el1, 8*16
    SAVE_SYS_PAIR tcr_el1, vbar_el1, 9*16
    SAVE_SYS_PAIR mair_el1, amair_el1, 10*16
    SAVE_SYS_PAIR tpidr_el0, tpidr_el1, 11*16
    SAVE_SYS_PAIR tpidrro_el0, contextidr_el1, 12*16
//    SAVE_SYS_PAIR par_el1, far_el1, 13*16
//    SAVE_SYS_PAIR afsr0_el1, afsr1_el1, 14*16

    stp x4, x5, [x0, #15*16]
    stp x6, x7, [x0, #16*16]
    stp x8, x9, [x0, #17*16]
    stp x10, x11, [x0, #18*16]
    stp x12, x13, [x0, #19*16]
    stp x14, x15, [x0, #20*16]
    stp x16, x17, [x0, #21*16]
    stp x18, x19, [x0, #22*16]
    stp x20, x21, [x0, #23*16]
    stp x22, x23, [x0, #24*16]
    stp x24, x25, [x0, #25*16]
    stp x26, x27, [x0, #26*16]
    stp x28, x29, [x0, #27*16]
    str x30, [x0, #28*16]
.endm

.macro MONITOR_RESTORE_STATE
    RESTORE_SYS_PAIR elr_el3, spsr_el3, 0*16
    RESTORE_SYS_PAIR elr_el1, spsr_el1, 1*16
    RESTORE_SYS_PAIR esr_el1, sp_el1, 2*16
    RESTORE_SYS_PAIR spsel, sp_el0, 3*16
//    RESTORE_SYS_PAIR spsr_abt, spsr_und, 4*16
//    RESTORE_SYS_PAIR spsr_irq, spsr_fiq, 5*16
    RESTORE_SYS_PAIR sctlr_el1, actlr_el1, 6*16
    RESTORE_SYS_PAIR cpacr_el1, csselr_el1, 7*16
    RESTORE_SYS_PAIR ttbr0_el1, ttbr1_el1, 8*16
    RESTORE_SYS_PAIR tcr_el1, vbar_el1, 9*16
    RESTORE_SYS_PAIR mair_el1, amair_el1, 10*16
    RESTORE_SYS_PAIR tpidr_el0, tpidr_el1, 11*16
    RESTORE_SYS_PAIR tpidrro_el0, contextidr_el1, 12*16
//    RESTORE_SYS_PAIR par_el1, far_el1, 13*16
//    RESTORE_SYS_PAIR afsr0_el1, afsr1_el1, 14*16

    ldp x4, x5, [x0, #15*16]
    ldp x6, x7, [x0, #16*16]
    ldp x8, x9, [x0, #17*16]
    ldp x10, x11, [x0, #18*16]
    ldp x12, x13, [x0, #19*16]
    ldp x14, x15, [x0, #20*16]
    ldp x16, x17, [x0, #21*16]
    ldp x18, x19, [x0, #22*16]
    ldp x20, x21, [x0, #23*16]
    ldp x22, x23, [x0, #24*16]
    ldp x24, x25, [x0, #25*16]
    ldp x26, x27, [x0, #26*16]
    ldp x28, x29, [x0, #27*16]
    ldr x30, [x0, #28*16]
.endm

.globl monitor_restore_state
monitor_restore_state:
    str x30, [sp, #-8]!
    MONITOR_RESTORE_STATE
    ldr x30, [sp], #8
    ret

.globl monitor_switch
monitor_switch:
    stp x0, x1, [sp, #-16]!     /* Save the input regs x0:x3 */
    stp x2, x3, [sp, #-16]!     /* Save the input regs x0:x3 */
    mrs x3, scr_el3
    and x2, x3, #0x1
    cbz x2, switch_to_nsec
switch_to_sec:
    adr x0, nsec_state
    MONITOR_SAVE_STATE
    adr x0, sec_state
    MONITOR_RESTORE_STATE
    b switch_state
switch_to_nsec:
    adr x0, sec_state
    MONITOR_SAVE_STATE
    adr x0, nsec_state
    MONITOR_RESTORE_STATE
switch_state:
    eor x3, x3, #0x1
    msr scr_el3, x3
    ldp x2, x3, [sp], #16
    ldp x0, x1, [sp], #16
    eret
.end
