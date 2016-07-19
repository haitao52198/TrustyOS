#define __ASSEMBLY__
#include "cpu.h"
#include "exception.h"
#undef __ASSEMBLY__

.section .vectors
.align 12                   // Align to vector table size (0x800)
.globl el3_vectors
el3_vectors:
.word 0                     // Add padding to force the below alignment
.align 9                    // Force these vectors to 0x400 alignment
el3_sync_exception_current:
    str x30, [sp, #-8]!
    stp x2, x3, [sp, #-16]!
    stp x0, x1, [sp, #-16]!
    mrs x0, esr_el3
    mov x1, #0xffffff
    and x1, x1, x0
    lsr x0, x0, #26
    mrs x2, far_el3
    mrs x3, elr_el3
    bl el3_handle_exception
    ldp x0, x1, [sp], #16
    ldp x2, x3, [sp], #16
    ldr x30, [sp], #8
    eret
.align 10                   // Force these vectors to 0x400 alignment
el3_sync_exception_lower64:
    str x30, [sp, #-8]!
    stp x2, x3, [sp, #-16]!
    mrs x2, spsr_el3    /* Nested EL3 exceptions will overwrite the SPSR */
    str x2, [sp, #-8]!  /* Save the SPSR so we can restore it later */
    stp x0, x1, [sp, #-16]!
    mrs x2, far_el3
    mrs x3, elr_el3
    mrs x0, esr_el3
    mov x1, #0xffffff
    and x1, x1, x0
    lsr x0, x0, #26
    cmp x0, #EC_SMC64
    b.eq el3_sync_exception_lower64_smc
    cmp x0, #EC_SMC32
    b.eq el3_sync_exception_lower64_smc
    bl el3_handle_exception
    ldp x0, x1, [sp], #16
    ldr x2, [sp], #8
    msr spsr_el3, x2        /* Restore the SPSR in case it was destroyed */
    ldp x2, x3, [sp], #16
    ldr x30, [sp], #8
    b el3_sync_exception_lower64_done
el3_sync_exception_lower64_smc:
    ldp x0, x1, [sp]        /* Fetch our inputs as SMC args */
    bl el3_handle_smc
    ldp x2, x1, [sp], #16   /* We don't want to overwrite x0, so use x2 */
    ldr x2, [sp], #8
    msr spsr_el3, x2        /* Restore the SPSR in case it was destroyed */
    ldp x2, x3, [sp], #16   /* We can throw away the old x0, and restore x2 */
    ldr x30, [sp], #8
    cbz x0, el3_sync_exception_lower64_done
    b monitor_switch        /* This never returns, erets */
el3_sync_exception_lower64_done:
    eret
.align 7
el3_serr_exception:
	b	el3_serr_exception
.align 7
el3_irq_exception:
	b	el3_irq_exception
.align 7
el3_fiq_exception:
	b	el3_fiq_exception

.end
