/*
 * Copyright (c) 2013, Google Inc. All rights reserved
 * Copyright (c) 2012-2013, NVIDIA CORPORATION. All rights reserved
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

#include <uthread.h>
#include <stdlib.h>
#include <assert.h>
#include <debug.h>
#include <arch.h>
#include <arch/arm.h>
#include <arch/arm/mmu.h>
#include <arch/uthread_mmu.h>
#include <uthread.h>

#define PAGE_MASK	(PAGE_SIZE - 1)

void arch_uthread_init()
{
}

void arch_uthread_startup(void)
{
	struct uthread *ut = (struct uthread *) tls_get(TLS_ENTRY_UTHREAD);
	register uint32_t sp_usr asm("r2") = ROUNDDOWN(ut->start_stack, 8);
	register uint32_t entry asm("r3") = ut->entry;

	__asm__ volatile(
		"mov	r0, #0\n"
		"mov	r1, #0\n"
		"stmdb	sp, {%[stack], %[entry]}	\n"
		"ldmdb	sp, {r13, r14}^		\n"
		"msr	spsr_fxsc, #0x10	\n"
		"movs	pc, %[entry]			\n"
		:
		: [stack]"r" (sp_usr), [entry]"r" (entry)
		: "r0", "r1", "memory"
	);
}

void arch_uthread_context_switch(struct uthread *old_ut, struct uthread *new_ut)
{
	paddr_t pgd;

	if (old_ut) {
		arm_write_ttbr0(arm_read_ttbr1());
		ISB;
	}

	if (new_ut) {
		pgd = kvaddr_to_paddr(new_ut->page_table);
		arm_write_contextidr(new_ut->arch.asid);
		ISB;
		arm_write_ttbr0(pgd | MMU_TTBRx_FLAGS);
		ISB;
	}
}

status_t arch_uthread_create(struct uthread *ut)
{
	status_t err = NO_ERROR;

	ut->arch.asid = ut->id;
	ut->arch.uthread = ut;
	return err;
}

void arch_uthread_free(struct uthread *ut)
{
}

status_t arch_uthread_map(struct uthread *ut, struct uthread_map *mp)
{
	addr_t vaddr, paddr;
	u_int pg, l1_flags, l2_flags;
	status_t err = NO_ERROR;

	if (mp->size > MAX_USR_VA || mp->vaddr > (MAX_USR_VA - mp->size)) {
		dprintf(CRITICAL, "virtual address exceeds max: 0x%x\n",
			MAX_USR_VA);

		err = ERR_INVALID_ARGS;
		goto done;
	}

	ASSERT(!(mp->size & PAGE_MASK));

	l1_flags = (mp->flags & UTM_NS_MEM) ? MMU_MEMORY_L1_PAGETABLE_NON_SECURE : 0;

	l2_flags = (mp->flags & UTM_W) ? MMU_MEMORY_L2_AP_P_RW_U_RW :
					  MMU_MEMORY_L2_AP_P_RO_U_RO;
	if (mp->flags & UTM_IO) {
		l2_flags |= MMU_MEMORY_L2_TYPE_STRONGLY_ORDERED;
	} else {
		/* shareable */
		l2_flags |= MMU_MEMORY_L2_SHAREABLE;

		/* inner cacheable (cb) */
		l2_flags |= MMU_MEMORY_SET_L2_INNER(MMU_MEMORY_WRITE_BACK_ALLOCATE);

		/* outer cacheable (tex) */
		l2_flags |= (MMU_MEMORY_SET_L2_CACHEABLE_MEM |
				MMU_MEMORY_SET_L2_OUTER(MMU_MEMORY_WRITE_BACK_ALLOCATE));
	}

	for (pg = 0; pg < (mp->size / PAGE_SIZE); pg++) {
		if (mp->flags & UTM_PHYS_CONTIG)
			paddr = mp->pfn_list[0] + (pg * PAGE_SIZE);
		else
			paddr = mp->pfn_list[pg];

		if (paddr & PAGE_MASK) {
			err = ERR_INVALID_ARGS;
			goto err_undo_maps;
		}

		vaddr = mp->vaddr + (pg * PAGE_SIZE);

		err = arm_uthread_mmu_map(ut, paddr, vaddr,
					l1_flags, l2_flags);

		if (err)
			goto err_undo_maps;
	}

	return NO_ERROR;

err_undo_maps:
	for(u_int p = 0; p < pg; p++) {
		arm_uthread_mmu_unmap(ut,
			mp->vaddr + (p * PAGE_SIZE));
	}
done:
	return err;
}

status_t arch_uthread_unmap(struct uthread *ut, struct uthread_map *mp)
{
	addr_t vaddr;
	u_int pg;
	status_t err = NO_ERROR;

	for (pg = 0; pg < (mp->size / PAGE_SIZE); pg++) {
		vaddr = mp->vaddr + (pg * PAGE_SIZE);
		err = arm_uthread_mmu_unmap(ut, vaddr);

		if (err)
			goto done;
	}

done:
	return err;
}

#if UTHREAD_WITH_MEMORY_MAPPING_SUPPORT
static void arm_write_v2p(ext_vaddr_t vaddr, v2p_t v2p)
{
	switch(v2p) {
		case ATS1CUR:
			__asm__ volatile(
				"mcr	p15, 0, %0, c7, c8, 2	\n"
				: : "r"(vaddr)
			);
			break;

		case ATS1CUW:
			__asm__ volatile(
				"mcr	p15, 0, %0, c7, c8, 3	\n"
				: : "r"(vaddr)
			);
			break;

		case ATS1CPR:
			__asm__ volatile(
				"mcr	p15, 0, %0, c7, c8, 0	\n"
				: : "r"(vaddr)
			);
			break;

		/* NS vaddrs can be 64-bits wide. Pass the lower bits as an arg
		 * to mcr and higher bits in a separate register. The monitor
		 * assembles them into a 64-bit register before translation
		 */

		case ATS12NSOUR:
			__asm__ volatile(
				"mov	r0, %H0	\n"
				"mcr	p15, 0, %0, c7, c8, 6	\n"
				: : "r"(vaddr) : "r0"
			);
			break;

		case ATS12NSOUW:
			__asm__ volatile(
				"mov	r0, %H0	\n"
				"mcr	p15, 0, %0, c7, c8, 7	\n"
				: : "r"(vaddr) : "r0"
			);
			break;

		case ATS12NSOPR:
			__asm__ volatile(
				"mov	r0, %H0	\n"
				"mcr	p15, 0, %0, c7, c8, 4	\n"
				: : "r"(vaddr) : "r0"
			);
			break;

		default:
			dprintf(CRITICAL, "%s unknown V2P type: %d\n",
					__func__, v2p);
	}
}

static uint64_t arm_read_par64(void)
{
	uint32_t lower, higher;

	__asm__ volatile(
		"mrc	p15, 0, %0, c7, c4, 0	\n"
		"tst	%0, #(1 << 11)		@ LPAE / long desc format\n"
		"moveq	%1, #0			\n"
		"mrrcne	p15, 0, %0, %1, c7	\n"
		 :"=r"(lower), "=r"(higher) : :
	);

	return ((uint64_t)higher << 32) | lower;
}


/* TEX[1:0] and CB mapping:
 *
 * 00	Non-cacheable
 * 01	Write-back, Write-allocate
 * 10	Write-through, no Write-allocate
 * 11	Write-back, no Write-allocate
 *
 * MAIR mappings
 *
 * 0100		Non-cacheable
 * 00RW,10RW	Write-through
 * 01RW,11RW	Write-back
 *
 */
static u_int mair_to_cb [] = {
	/* 0000 */	0x4,	/* Error! Device memory */
	/* 0001 */	0x2,	/* write through transient, write allocate */
	/* 0010 */	0x2,	/* write through transient, read allocate */
	/* 0011 */	0x2,	/* write through transient, read/write allocate */
	/* 0100 */	0x0,	/* Non-cacheable */
	/* 0101 */	0x1,	/* write back transient, write allocate */
	/* 0110 */	0x3,	/* write back transient, read allocate */
	/* 0111 */	0x1,	/* write back transient, read/write allocate */
	/* 1000 */	0x2,	/* write through non-transient */
	/* 1001 */	0x2,	/* write through non-transient, write allocate */
	/* 1010 */	0x2,	/* write through non-transient, read allocate */
	/* 1011 */	0x2,	/* write through non-transient, read/write allocate */
	/* 1100 */	0x3,	/* write back non-transient */
	/* 1101 */	0x1,	/* write back non-transient, write allocate */
	/* 1110 */	0x3,	/* write back non-transient, read allocate */
	/* 1111 */	0x1,	/* write back non-transient, read/write allocate */
};

/* Translate vaddr from current context and map into target uthread */
status_t arch_uthread_translate_map(struct uthread *ut_target,
				    ext_vaddr_t vaddr_src,
				    vaddr_t vaddr_target, paddr_t *pfn_list,
				    uint32_t npages, u_int flags, bool ns_src,
				    uint64_t *ns_pte_list)
{
	u_int type, pg;
	ext_vaddr_t vs;
	vaddr_t vt;
	u_int l1_flags, l2_flags;
	status_t err = NO_ERROR;
	u_int first_inner, first_outer, first_shareable;

	type = (flags & UTM_W) ?
		(ns_src ? ATS12NSOUW : ATS1CUW) :
		(ns_src ? ATS12NSOUR : ATS1CUR);

	l1_flags = (flags & UTM_NS_MEM) ? MMU_MEMORY_L1_PAGETABLE_NON_SECURE : 0;
	l2_flags = (flags & UTM_W) ? MMU_MEMORY_L2_AP_P_RW_U_RW :
				     MMU_MEMORY_L2_AP_P_RO_U_RO;

	vs = vaddr_src;
	vt = vaddr_target;
	for (pg = 0; pg < npages; pg++, vs += PAGE_SIZE, vt += PAGE_SIZE) {
		u_int inner, outer, shareable;
		uint64_t par;
		uint64_t physaddr;

		if (ns_src) {
			/* Get physaddr and attr from the list of ptes
			 *
			 * !!! This code only works if ptes are ARM v8
			 *     VMSAv8-64 level 3 page table entries
			 *     describing 4K granular pages
			 * !!!
			 * TODO: Add support for other page table formats.
			 */
			uint64_t pte = ns_pte_list[pg];
			if ((flags & UTM_W) && !NS_PTE_AP_U_RW(pte)) {
				dprintf(CRITICAL, "%s: Making a writable mapping "
						"to NS read-only page (pte:0x%llx)!\n", __func__, pte);
				err = ERR_NOT_VALID;
				goto err_out;
			}

			physaddr = NS_PTE_PHYSADDR(pte);

			outer = mair_to_cb[NS_PTE_ATTR_OUTER(pte)];
			if (outer & 0x4) {
				dprintf(CRITICAL, "%s: Unknown outer cache attributes 0x%x\n",
						__func__, outer);
				err = ERR_NOT_VALID;
				goto err_out;
			}

			inner = mair_to_cb[NS_PTE_ATTR_INNER(pte)];
			if (inner & 0x4) {
				dprintf(CRITICAL, "%s: Unknown inner cache attributes 0x%x\n",
						__func__, inner);
				err = ERR_NOT_VALID;
				goto err_out;
			}

			shareable = NS_PTE_ATTR_SHAREABLE(pte);
		} else {
			arch_disable_ints();
			arm_write_v2p(vs, type);
			par = arm_read_par64();
			arch_enable_ints();

			if (par & PAR_ATTR_FAULTED) {
				dprintf(CRITICAL,
					"failed %s user read V2P (v = 0x%016llx, par = 0x%016llx)\n",
					(flags & UTM_NS_MEM) ? "NS" : "SEC",
					vs, par);
				err = ERR_NOT_VALID;
				goto err_out;
			}

			if (par & PAR_ATTR_LPAE) {
				inner = PAR_LDESC_ATTR_INNER(par);
				outer = PAR_LDESC_ATTR_OUTER(par);
				shareable = PAR_LDESC_ATTR_SHAREABLE(par);
				physaddr = PAR_LDESC_ALIGNED_PA(par);
			} else {
				inner = PAR_SDESC_ATTR_INNER(par);
				outer = PAR_LDESC_ATTR_OUTER(par);
				shareable = PAR_SDESC_ATTR_SHAREABLE(par);
				physaddr = PAR_SDESC_ALIGNED_PA(par);
			}
		}
#if !WITH_LPAE
		/* without LPAE enabled, we can't map memory beyond 4GB */
		if (physaddr >> 32) {
			dprintf(CRITICAL,
				"%s: physaddr (0x%016llx) >= 4GB and LPAE is unsupported\n",
				__func__, physaddr);
			err = ERR_NOT_SUPPORTED;
			goto err_out;
		}
#endif
		/*
		 * Set flags on first page and check for attribute
		 * consistency on subsequent pages
		 */
		if (pg == 0) {
			if (shareable)
				l2_flags |= MMU_MEMORY_L2_SHAREABLE;

			/* inner cacheable (cb) */
			l2_flags |= MMU_MEMORY_SET_L2_INNER(inner);

			/* outer cacheable (tex) */
			l2_flags |= (MMU_MEMORY_SET_L2_CACHEABLE_MEM |
					MMU_MEMORY_SET_L2_OUTER(outer));

			first_inner = inner;
			first_outer = outer;
			first_shareable = shareable;
		} else if (inner != first_inner || outer != first_outer ||
				shareable != first_shareable) {
			dprintf(CRITICAL,
				"%s: attribute inconsistency!\n", __func__);
			err = ERR_NOT_VALID;
			goto err_out;
		}
		pfn_list[pg] = physaddr;

		err = arm_uthread_mmu_map(ut_target, pfn_list[pg], vt,
				l1_flags, l2_flags);
		if (err != NO_ERROR)
			goto err_out;
	}

	return NO_ERROR;

err_out:
	while (vt > vaddr_target) {
		vt -= PAGE_SIZE;
		arm_uthread_mmu_unmap(ut_target, vt);
	}
	return err;
}
#endif
