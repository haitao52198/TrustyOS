#define __ASSEMBLY__
#include "cpu.h"
#include "exception.h"
#undef __ASSEMBLY__

.section .vectors
.align 12
.global el3_vectors
el3_vectors:
	b	.					/* Unused			*/
	b	.					/* Unused			*/
	b	el3_smc_vect 		/* Secure el3 call	*/
	b	el3_iabort_vect 	/* Prefetch abort 	*/
	b	el3_dabort_vect 	/* Data abort 		*/
	b	.					/* Unused			*/
	b	.					/* IRQ				*/
	b	.       			/* FIQ				*/

el3_smc_vect:
	srsdb sp!, #CPSR_M_MON
	mov r2, #0                      /* There is no FAR on an SVC */
	ldr r3, [sp, #16]               /* Jump back over the saved args for LR */
    bl el3_handle_smc
    cmp r0, #0
    beq el3_smc_vect_done
    b monitor_switch
el3_smc_vect_done:
    rfefd sp!

el3_iabort_vect:
	srsdb sp!, #CPSR_M_MON
    push {r0-r3}
	mov r0, #EC_IABORT				/* Treat ARMv7 iaborts as at same level */
	mrc p15, 0, r1, c5, c0, 1		/* ISS = IFSR on ARMv7 */
	mrc p15, 0, r2, c6, c0, 2		/* IFAR */
	ldr r3, [sp, #16]               /* Jump back over the saved args for LR */
    bl el3_handle_exception
    pop {r0-r3}
    rfefd sp!

el3_dabort_vect:
	srsdb sp!, #CPSR_M_MON
    push {r0-r3}
	mov r0, #EC_DABORT				/* Treat ARMv7 iaborts as at same level */
	mrc p15, 0, r1, c5, c0, 0		/* ISS = DFSR on ARMv7 */
	mrc p15, 0, r2, c6, c0, 0		/* DFAR */
	ldr r3, [sp, #16]               /* Jump back over the saved args for LR */
    bl el3_handle_exception
    pop {r0-r3}
    rfefd sp!
