/*
 * Copyright (c) 2013, Google Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include <asm.h>
#include <lib/sm/monitor.h>
#include <lib/sm/smcall.h>
#include <lib/sm/sm_err.h>

#include <kernel/vm.h>

/* sm_sched_nonsecure(uint32_t retval, smc32_args_t *args) */
FUNCTION(sm_sched_nonsecure)
	push	{r1, lr}

.Lfastcall_complete:
	mov	r1, r0

.Lreturn_sm_err:
	ldr	r0, =SMC_SC_NS_RETURN
	smc	#0

	tst	r0, #(1 << 30)		/* Check calling convention */
	movne	r1, #SM_ERR_NOT_SUPPORTED
	bne	.Lreturn_sm_err

	tst	r0, #(1 << 31)		/* Check if fastcall */
	beq	.Lstdcall		/* handle stdcall */

	push	{r0-r3}
	ubfxne	r0, r0, #24, #6		/* r0 = entity */
	ldrne	r14, =sm_fastcall_table
	ldrne	r14, [r14, r0, lsl #2]

	mov	r0, sp			/* r0 = smc_args_t* args */
	blx	r14
	add	sp, #(4 * SMC_NUM_ARGS)

	b	.Lfastcall_complete

.Lstdcall:
	pop	{r12, lr}
	stmia	r12, {r0-r3}
	bx	lr

FUNCTION(platform_reset)
#if WITH_LIB_SM_MONITOR
	cps	#MODE_MON
	bl	monitor_reset
	cps	#MODE_SVC
#endif
#if WITH_SMP
	/* figure out our cpu number */
	mrc     p15, 0, ip, c0, c0, 5 /* read MPIDR */

	/* mask off the bottom bits to test cluster number:cpu number */
	ubfx    ip, ip, #0, #SMP_CPU_ID_BITS

	/* if we're not cpu 0:0, jump back to arm_reset */
	cmp     ip, #0
	bne     arm_reset
#endif

	adr	sp, platform_reset	/* sp = paddr */
	ldr	ip, =platform_reset	/* ip = vaddr */
	sub	ip, sp, ip		/* ip = phys_offset */

	/* patch mmu_initial_mappings table */
	ldr	r5, =mmu_initial_mappings
	add	r5, r5, ip	/* r5 = _mmu_initial_mappings paddr */

.Lnext_entry:
	/* if size == 0, end of list */
	ldr	r4, [r5, #__MMU_INITIAL_MAPPING_SIZE_OFFSET]
	cmp	r4, #0
	beq	.Lall_done

	ldr	r4, [r5, #__MMU_INITIAL_MAPPING_FLAGS_OFFSET]
	tst	r4, #MMU_INITIAL_MAPPING_FLAG_DYNAMIC
	addeq	r5, #__MMU_INITIAL_MAPPING_SIZE
	beq	.Lnext_entry

	/* patching dynamic entry: r5 - points to entry to patch */
	/* r0 is memsize passed in by the bootloader */

	/* update size field of mmu_initial_mappings struct */
	str	r0, [r5, #__MMU_INITIAL_MAPPING_SIZE_OFFSET]

	/* calculate phys mem base */
	ldr	r4, =_start	/* r4 = _start vaddr */
	add     r4, r4, ip      /* r4 = _start paddr */

	/* update phys field of mmu_initial_mappings struct */
	str	r4, [r5, #__MMU_INITIAL_MAPPING_PHYS_OFFSET]

.Lall_done:
	b	arm_reset

FUNCTION(smc_fastcall_secure_monitor)
	ldrh	r1, [r0]	/* r1 = function# */
	ldr	r2, =sm_nr_fastcall_functions
	ldr	r2, [r2]
	cmp	r1, r2
	ldrlo	r2, =sm_fastcall_function_table
	ldrlo	r2, [r2, r1, lsl#2]
	rsblos	r1, r2, #1	/* This will set the same flags as the cmp above */
	ldrhs	r2, =smc_undefined
	bx	r2
