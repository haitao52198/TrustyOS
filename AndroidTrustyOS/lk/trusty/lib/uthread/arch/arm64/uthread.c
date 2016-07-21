/*
 * Copyright (c) 2013-2014, Google Inc. All rights reserved
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
#include <arch/arm64.h>
#include <arch/arm64/mmu.h>
#include <arch/uthread_mmu.h>
#include <uthread.h>
#include <string.h>
#include <trace.h>

#define LOCAL_TRACE 0

#define USER_PAGE_MASK	(USER_PAGE_SIZE - 1)

void arch_uthread_init(void)
{
}

void arch_uthread_startup(void)
{
	struct uthread *ut = (struct uthread *) tls_get(TLS_ENTRY_UTHREAD);
	register uint64_t sp_usr asm("x2") = ROUNDDOWN(ut->start_stack, 8);
	register uint64_t entry asm("x3") = ut->entry;

	__asm__ volatile(
		"mov	x0, #0\n"
		"mov	x1, #0\n"
		"mov	x13, %[stack]\n" /* AArch32 SP_usr */
		"mov	x14, %[entry]\n" /* AArch32 LR_usr */
		"mov	x9, #0x10\n" /* Mode = AArch32 User */
		"msr	spsr_el1, x9\n"
		"msr	elr_el1, %[entry]\n"
		"eret\n"
		:
		: [stack]"r" (sp_usr), [entry]"r" (entry)
		: "x0", "x1", "memory"
	);
}

void arch_uthread_context_switch(struct uthread *old_ut, struct uthread *new_ut)
{
	paddr_t pgd;

	if (old_ut && !new_ut) {
		ARM64_WRITE_SYSREG(tcr_el1, MMU_TCR_FLAGS_KERNEL);
	}

	if (new_ut) {
		pgd = vaddr_to_paddr(new_ut->page_table);
		ARM64_WRITE_SYSREG(ttbr0_el1, (paddr_t)new_ut->arch.asid << 48 | pgd);
		if (!old_ut)
			ARM64_WRITE_SYSREG(tcr_el1, MMU_TCR_FLAGS_USER);
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
	arm64_mmu_unmap(0, 1UL << MMU_USER_SIZE_SHIFT,
	                0, MMU_USER_SIZE_SHIFT,
	                MMU_USER_TOP_SHIFT, MMU_USER_PAGE_SIZE_SHIFT,
	                ut->page_table, ut->arch.asid);

	free(ut->page_table);
}

status_t arm64_uthread_allocate_page_table(struct uthread *ut)
{
	size_t page_table_size;

	page_table_size = MMU_USER_PAGE_TABLE_ENTRIES_TOP * sizeof(pte_t);

	ut->page_table = memalign(page_table_size, page_table_size);
	if (!ut->page_table)
		return ERR_NO_MEMORY;

	memset(ut->page_table, 0, page_table_size);

	LTRACEF("id %d, user page table %p, size %ld\n",
	        ut->id, ut->page_table, page_table_size);

	return NO_ERROR;
}

status_t arch_uthread_map(struct uthread *ut, struct uthread_map *mp)
{
	paddr_t pg, pte_attr;
	size_t entry_size;
	status_t err = NO_ERROR;

	if (!ut->page_table) {
		err = arm64_uthread_allocate_page_table(ut);
		if (err)
			return err;
	}

	ASSERT(!(mp->size & USER_PAGE_MASK));

	pte_attr = MMU_PTE_ATTR_NON_GLOBAL | MMU_PTE_ATTR_AF;

	pte_attr |= (mp->flags & UTM_NS_MEM) ? MMU_PTE_ATTR_NON_SECURE : 0;

	pte_attr |= (mp->flags & UTM_W) ? MMU_PTE_ATTR_AP_P_RW_U_RW :
	                                  MMU_PTE_ATTR_AP_P_RO_U_RO;
	if (mp->flags & UTM_IO) {
		pte_attr |= MMU_PTE_ATTR_STRONGLY_ORDERED;
	} else {
		/* shareable */
		pte_attr |= MMU_PTE_ATTR_SH_INNER_SHAREABLE;

		/* use same cache attributes as kernel */
		pte_attr |= MMU_PTE_ATTR_NORMAL_MEMORY;
	}

	entry_size = (mp->flags & UTM_PHYS_CONTIG) ? mp->size : USER_PAGE_SIZE;
	for (pg = 0; pg < (mp->size / entry_size); pg++) {
		err = arm64_mmu_map(mp->vaddr + pg * entry_size,
		                    mp->pfn_list[pg], entry_size, pte_attr,
		                    0, MMU_USER_SIZE_SHIFT, MMU_USER_TOP_SHIFT,
		                    MMU_USER_PAGE_SIZE_SHIFT,
		                    ut->page_table, ut->arch.asid);
		if (err)
			goto err_undo_maps;
	}

	return NO_ERROR;

err_undo_maps:
	for(u_int p = 0; p < pg; p++) {
		arm64_mmu_unmap(mp->vaddr + p * entry_size, entry_size,
		                0, MMU_USER_SIZE_SHIFT,
		                MMU_USER_TOP_SHIFT, MMU_USER_PAGE_SIZE_SHIFT,
		                ut->page_table, ut->arch.asid);
	}

	return err;
}

status_t arch_uthread_unmap(struct uthread *ut, struct uthread_map *mp)
{
	return arm64_mmu_unmap(mp->vaddr, mp->size, 0, MMU_USER_SIZE_SHIFT,
	                       MMU_USER_TOP_SHIFT, MMU_USER_PAGE_SIZE_SHIFT,
	                       ut->page_table, ut->arch.asid);
}

#ifdef UTHREAD_WITH_MEMORY_MAPPING_SUPPORT

#define ARM64_WRITE_AT(reg, val) \
({ \
    __asm__ volatile("at " #reg ", %0" :: "r" (val)); \
    ISB; \
})

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
	u_int pg;
	ext_vaddr_t vs;
	vaddr_t vt;
	pte_t pte_attr;
	status_t err = NO_ERROR;
	u_int first_inner = ~0, first_outer = ~0, first_shareable = ~0;

	pte_attr = MMU_PTE_ATTR_NON_GLOBAL | MMU_PTE_ATTR_AF;
	if (flags & UTM_NS_MEM)
		pte_attr |= MMU_PTE_ATTR_NON_SECURE;
	if (flags & UTM_W)
		pte_attr |= MMU_PTE_ATTR_AP_P_RW_U_RW;
	else
		pte_attr |= MMU_PTE_ATTR_AP_P_RO_U_RO;

	vs = vaddr_src;
	vt = vaddr_target;
	for (pg = 0; pg < npages; pg++, vs += USER_PAGE_SIZE, vt += USER_PAGE_SIZE) {
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
						"to NS read-only page!\n", __func__);
				goto err_out;
			}

			physaddr = NS_PTE_PHYSADDR(pte);

			dprintf(SPEW, "%s: physaddr 0x%llx cache attributes 0x%llx, pte 0x%llx\n",
					__func__, physaddr, NS_PTE_ATTR_MAIR(pte), pte);

			outer = mair_to_cb[NS_PTE_ATTR_OUTER(pte)];
			if (outer & 0x4) {
				dprintf(CRITICAL, "%s: Unknown outer cache attributes 0x%x\n",
						__func__, outer);
				goto err_out;
			}

			inner = mair_to_cb[NS_PTE_ATTR_INNER(pte)];
			if (inner & 0x4) {
				dprintf(CRITICAL, "%s: Unknown inner cache attributes 0x%x\n",
						__func__, inner);
				goto err_out;
			}

			shareable = NS_PTE_ATTR_SHAREABLE(pte);
		} else {
			arch_disable_ints();
			if (flags & UTM_W)
				ARM64_WRITE_AT(S1E0W, vs);
			else
				ARM64_WRITE_AT(S1E0R, vs);
			par = ARM64_READ_SYSREG(par_el1);
			arch_enable_ints();

			if (par & PAR_ATTR_FAULTED) {
				dprintf(CRITICAL,
					"failed %s user read V2P (v = 0x%016llx, par = 0x%016llx)\n",
					(flags & UTM_NS_MEM) ? "NS" : "SEC",
					vs, par);
				err = ERR_NOT_VALID;
				goto err_out;
			}

			inner = PAR_LDESC_ATTR_INNER(par);
			outer = PAR_LDESC_ATTR_OUTER(par);
			shareable = PAR_LDESC_ATTR_SHAREABLE(par);
			physaddr = PAR_LDESC_ALIGNED_PA(par);
		}
		/*
		 * Set flags on first page and check for attribute
		 * consistency on subsequent pages
		 */
		if (pg == 0) {
			if (shareable)
				pte_attr |= MMU_PTE_ATTR_SH_INNER_SHAREABLE;

			/* inner cacheable (cb) */
			/* outer cacheable (tex) */
			if (inner | outer)
				pte_attr |= MMU_PTE_ATTR_NORMAL_MEMORY;

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

		if (ns_src) {
			dprintf(SPEW, "%s: vt 0x%lx, physaddr 0x%llx, select pte_attr 0x%llx\n",
					__func__, vt, physaddr, pte_attr);
		}

		err = arm64_mmu_map(vt, pfn_list[pg], USER_PAGE_SIZE, pte_attr,
		                    0, MMU_USER_SIZE_SHIFT, MMU_USER_TOP_SHIFT,
		                    MMU_USER_PAGE_SIZE_SHIFT,
		                    ut_target->page_table, ut_target->arch.asid);
		if (err != NO_ERROR)
			goto err_out;
	}

err_out:
	return err;
}
#endif
