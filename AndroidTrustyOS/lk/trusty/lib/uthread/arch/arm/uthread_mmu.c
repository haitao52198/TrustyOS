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

#include <err.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <arch.h>
#include <arch/arm.h>
#include <arch/arm/mmu.h>
#include <arch/uthread_mmu.h>
#include <lk/init.h>
#include <uthread.h>

static void arm_uthread_mmu_init(uint level)
{
	uint32_t cur_ttbr0;

	ASSERT(MEMBASE > MAX_USR_VA);

	cur_ttbr0 = arm_read_ttbr0();

	/* push out kernel mappings to ttbr1 */
	arm_write_ttbr1(cur_ttbr0);

	/* setup a user-kernel split */
	arm_write_ttbcr(MMU_MEMORY_TTBCR_N);

	arm_invalidate_tlb_global();
}

LK_INIT_HOOK_FLAGS(libuthreadarmmmu, arm_uthread_mmu_init,
                   LK_INIT_LEVEL_ARCH_EARLY, LK_INIT_FLAG_ALL_CPUS);

static u_int *arm_uthread_mmu_alloc_pgtbl(pgtbl_lvl_t type)
{
	u_int size;
	u_int *pgtable = NULL;

	switch (type) {
	case PGTBL_LEVEL_1_USER:
		size = MMU_MEMORY_TTBR0_L1_SIZE;
		break;
	case PGTBL_LEVEL_1_PRIV:
		size = MMU_MEMORY_TTBR1_L1_SIZE;
		break;
	case PGTBL_LEVEL_2:
		size = MMU_MEMORY_TTBR_L2_SIZE;
		break;
	default:
		dprintf(CRITICAL, "unrecognized pgtbl_type %d\n", type);
		return pgtable;
	}
	pgtable = memalign(size, size);
	if (pgtable)
		memset(pgtable, 0, size);
	return pgtable;
}

status_t arm_uthread_mmu_map(uthread_t *ut, paddr_t paddr,
		vaddr_t vaddr, uint l1_flags, uint l2_flags)
{
	uint32_t *page_table;
	u_int *level_2;
	paddr_t level_2_paddr;
	u_int idx;
	status_t err = NO_ERROR;

	if (ut->page_table == NULL) {
		ut->page_table = arm_uthread_mmu_alloc_pgtbl(PGTBL_LEVEL_1_USER);
		if (ut->page_table == NULL) {
			dprintf(CRITICAL,
				"unable to allocate LEVEL_1 page table\n");
			err = ERR_NO_MEMORY;
			goto done;
		}
	}

	page_table = (uint32_t *)(ut->page_table);
	ASSERT(page_table);

	idx = vaddr >> 20;
	idx &= MMU_MEMORY_TTBR0_L1_INDEX_MASK;

	level_2_paddr = page_table[idx] & ~(MMU_MEMORY_TTBR_L2_SIZE - 1);
	if (!level_2_paddr) {
		/* alloc level 2 page table */
		level_2 = arm_uthread_mmu_alloc_pgtbl(PGTBL_LEVEL_2);
		if (level_2 == NULL) {
			dprintf(CRITICAL, "unable to allocate LEVEL_2 page table\n");
			err = ERR_NO_MEMORY;
			goto done;
		}

		/* install in level_1 */
		page_table[idx] = kvaddr_to_paddr(level_2);
		page_table[idx] |= ((MMU_MEMORY_DOMAIN_MEM << 5) | l1_flags | 0x1);
	} else {
		level_2 = paddr_to_kvaddr(level_2_paddr);
	}

	idx = vaddr >> 12;
	idx &= MMU_MEMORY_TTBR_L2_INDEX_MASK;

	ASSERT(!level_2[idx]);

	/* install level_2 4K entries */
	level_2[idx] = paddr;
	level_2[idx] |= (MMU_MEMORY_L2_NON_GLOBAL | l2_flags | 0x2);

done:
	return err;
}

status_t arm_uthread_mmu_unmap(uthread_t *ut, vaddr_t vaddr)
{
	uint32_t *page_table;
	u_int *level_2;
	paddr_t level_2_paddr;
	u_int idx;
	status_t err = NO_ERROR;

	page_table = (uint32_t *)(ut->page_table);
	if (!page_table) {
		err = ERR_INVALID_ARGS;
		goto done;
	}

	idx = vaddr >> 20;
	idx &= MMU_MEMORY_TTBR0_L1_INDEX_MASK;

	level_2_paddr = page_table[idx] & ~(MMU_MEMORY_TTBR_L2_SIZE - 1);
	if (!level_2_paddr) {
		err = ERR_INVALID_ARGS;
		goto done;
	}
	else {
		level_2 = paddr_to_kvaddr(level_2_paddr);
	}

	idx = vaddr >> 12;
	idx &= MMU_MEMORY_TTBR_L2_INDEX_MASK;

	level_2[idx] = 0;	/* invalid entry */
	DSB;
	arm_invalidate_tlb_mva_asid(vaddr, ut->arch.asid);

done:
	return err;
}
