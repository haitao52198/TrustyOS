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
#include <arch/asm_macros.h>
#include <lib/sm/monitor.h>
#include <lib/sm/smcall.h>
#include <lib/sm/sm_err.h>

#include <kernel/vm.h>

FUNCTION(sm_sched_nonsecure)
	push	x1, x30
.Lfastcall_complete:
	mov	x1, x0
.Lreturn_sm_err:
	ldr	x0, =SMC_SC_NS_RETURN
	mov	x2, xzr
	mov	x3, xzr
	smc	#0

	tbnz	x0, #30, .Lsm_err_not_supported /* Check calling convention */
	tbz	x0, #31, .Lnot_fast_call

	/* fastcall */
	sub	sp, sp, #(4 * SMC_NUM_ARGS) /* allocate smc32_args_t on stack */
	stp	w0, w1, [sp]
	stp	w2, w3, [sp, #4 * 2]

	ubfx	x0, x0, #24, #6		/* x0 = entity */
	ldr	x9, =sm_fastcall_table
	ldr	x9, [x9, x0, lsl #3]

	mov	x0, sp			/* x0 = smc_args_t* args */
	blr	x9
	add	sp, sp, #(4 * SMC_NUM_ARGS)
	b	.Lfastcall_complete

.Lnot_fast_call:
	pop	x9, x30
	stp	w0, w1, [x9], #8
	stp	w2, w3, [x9], #8
	ret

.Lsm_err_not_supported:
	mov	x1, #SM_ERR_NOT_SUPPORTED
	b	.Lreturn_sm_err

.Lsm_err_busy:
	mov	x1, #SM_ERR_BUSY
	b	.Lreturn_sm_err

FUNCTION(smc_fastcall_secure_monitor)
	ldrh	w1, [x0]	/* r1 = function# */
	ldr	x2, =sm_nr_fastcall_functions
	ldr	w2, [x2]
	cmp	w1, w2
	bhs	.Lfastcall_smc_undefined
	ldr	x2, =sm_fastcall_function_table
	ldr	x2, [x2, x1, lsl#3]
	cbz	x2, .Lfastcall_smc_undefined
	br	x2
.Lfastcall_smc_undefined:
	ldr	x2, =smc_undefined
	br	x2
