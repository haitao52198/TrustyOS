#define __ASSEMBLY__
#include "smc.h"
#include "cpu.h"
#undef __ASSEMBLY__

.section .text

.globl monitor_restore_state
monitor_restore_state:
    push {lr}
    cps #CPSR_M_SYS
    ldmia r0!, {sp, lr}

    cps #CPSR_M_IRQ
    ldmia r0!, {r3, sp, lr}
	msr	spsr, r3

    cps #CPSR_M_SVC
    ldmia r0!, {r3, sp, lr}
	msr	spsr, r3

    cps #CPSR_M_ABT
    ldmia r0!, {r3, sp, lr}
	msr	spsr, r3

    cps #CPSR_M_UND
    ldmia r0!, {r3, sp, lr}
	msr	spsr, r3

    cps #CPSR_M_MON
    ldmia r0!, {r1, r2}
    ldmia r0!, {r4-r12}

    pop {r3}
    mov r0, r2
    push {r0, r1}
    bx r3

.globl monitor_save_state
monitor_save_state:
    push {lr}
    cps #CPSR_M_SYS
    stmia r0!, {sp, lr}

    cps #CPSR_M_IRQ
    mrs r3, spsr
    stmia r0!, {r3, sp, lr}

    cps #CPSR_M_SVC
    mrs r3, spsr
    stmia r0!, {r3, sp, lr}

    cps #CPSR_M_ABT
    mrs r3, spsr
    stmia r0!, {r3, sp, lr}

    cps #CPSR_M_UND
    mrs r3, spsr
    stmia r0!, {r3, sp, lr}

    cps #CPSR_M_MON
    stmia r0!, {r1, r2}
    stmia r0!, {r4-r12}

    pop {lr}
    bx lr

.globl monitor_switch
/* This function performs the secure state world switch.
 * We start by popping the exception stack details from the stack.  It is
 * expected that these are the most recent on the stack on entry. We reverse
 * the register priority when filling in r1, so they are in register order for
 * the state save buffer.
 * State saves are performed in secure mode regardless of whether we are going
 * to or from secure state.
 */
monitor_switch:
	pop {r2, r3}					/* Pop the exception LR & SPSR */
    push {r0, r1}
	mov r1, r3
    mrc p15, 0, r3, c1, c1, 0       /* Read SCR */
    and r3, r3, #0x1                /* Mask off NS bit */
    cmp r3, #0
    beq switch_to_nsec
switch_to_sec:
    mrc p15, 0, r3, c1, c1, 0       /* Read SCR */
    bic r3, r3, #SCR_NS
    mcr p15, 0, r3, c1, c1, 0       /* Write NS disabled SCR */
    ldr r0, =nsec_state
    bl monitor_save_state
    ldr r0, =sec_state
    bl monitor_restore_state
    b switch_state
switch_to_nsec:
    ldr r0, =sec_state
    bl monitor_save_state
    ldr r0, =nsec_state
    bl monitor_restore_state
    mrc p15, 0, r3, c1, c1, 0       /* Read SCR */
    orr r3, r3, #SCR_NS
    mcr p15, 0, r3, c1, c1, 0       /* Write NS enabled SCR */
switch_state:
    pop {r2, r3}
    pop {r0, r1}
    push {r2, r3}
    rfefd sp


