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
#ifndef __LIB_UTHREAD_ARCH_ARM_MMU_H
#define __LIB_UTHREAD_ARCH_ARM_MMU_H

#include <arch.h>
#include <uthread.h>

#define MMU_MEMORY_TTBCR_N		7

#define MMU_MEMORY_TTBR0_L1_INDEX_BITS	(((31 - MMU_MEMORY_TTBCR_N) - 20) + 1)
#define MMU_MEMORY_TTBR0_L1_INDEX_MASK	((1 << MMU_MEMORY_TTBR0_L1_INDEX_BITS) - 1)
#define MMU_MEMORY_TTBR0_L1_SIZE	(1 << (MMU_MEMORY_TTBR0_L1_INDEX_BITS + 2))

#define MMU_MEMORY_TTBR1_L1_INDEX_BITS	((31 - 20) + 1)
#define MMU_MEMORY_TTBR1_L1_INDEX_MASK	((1 << MMU_MEMORY_TTBR1_L1_INDEX_BITS) - 1)
#define MMU_MEMORY_TTBR1_L1_SIZE	(1 << (MMU_MEMORY_TTBR1_L1_INDEX_BITS + 2))

#define MMU_MEMORY_TTBR_L2_INDEX_BITS	((19 - 12) + 1)
#define MMU_MEMORY_TTBR_L2_INDEX_MASK	((1 << MMU_MEMORY_TTBR_L2_INDEX_BITS) - 1)
#define MMU_MEMORY_TTBR_L2_SIZE		(1 << (MMU_MEMORY_TTBR_L2_INDEX_BITS + 2))

#define MAX_USR_VA		((MMU_MEMORY_TTBR0_L1_SIZE / 4) * \
				((MMU_MEMORY_TTBR_L2_SIZE / 4) * PAGE_SIZE))

#if UTHREAD_WITH_MEMORY_MAPPING_SUPPORT
/* PAR register: common defines (between short/long desc) */
#define PAR_ATTR_FAULTED	(0x1 <<  0)
#define PAR_ATTR_LPAE		(0x1 << 11)

/* PAR register: short desc defines */
#define PAR_SDESC_ATTR_NON_SECURE	(0x1 << 9)
#define PAR_SDESC_ATTR_SHAREABLE(par)	(((par) >> 7) & 0x1)
#define PAR_SDESC_ATTR_INNER(par)	(((par) >> 4) & 0x7)
#define PAR_SDESC_ATTR_OUTER(par)	(((par) >> 2) & 0x3)

#define PAR_SDESC_ATTR_SSECTION		(0x1 << 1)
#define PAR_SDESC_ALIGNED_PA(par)	\
	(((par) & PAR_SDESC_ATTR_SSECTION) ? 	/* super-section */		\
		(((par) & 0xFF000000ULL) | (((par) & 0x00FF0000ULL) << 16)) : 	\
		((par) & ~PAGE_MASK))	/* section or large/small page */

/* PAR register: long desc defines */
#define PAR_LDESC_ATTR_NON_SECURE	(0x1 << 9)
#define PAR_LDESC_ATTR_SHAREABLE(par)	(((par) >> 7) & 0x3)
#define PAR_LDESC_ATTR_INNER(par)	(((par) >> 56) & 0xF)
#define PAR_LDESC_ATTR_OUTER(par)	(((par) >> 60) & 0xF)

#define PAR_LDESC_ALIGNED_PA(par)	((par) & 0xFFFFFF000ULL)

/* virt -> phys address translation args */
typedef enum {
	ATS1CUR,
	ATS1CUW,
	ATS12NSOUR,
	ATS12NSOUW,
	ATS1CPR,
	ATS12NSOPR,
} v2p_t;

/* 48-bit physical address 47:12 */
#define NS_PTE_PHYSADDR(pte)		((pte) & 0xFFFFFFFFF000ULL)

/* Access permissions AP[2:1]
 *	EL0	EL1
 * 00	None	RW
 * 01	RW	RW
 * 10	None	RO
 * 11	RO	RO
 */
#define NS_PTE_AP(pte)			(((pte) >> 6) & 0x3)
#define NS_PTE_AP_U_RW(pte)		(NS_PTE_AP(pte) == 0x1)

/* Shareablility attrs */
#define NS_PTE_ATTR_SHAREABLE(pte)	(((pte) >> 8) & 0x3)

/* cache attrs encoded in the top bits 55:49 of the PTE*/
#define NS_PTE_ATTR_MAIR(pte)	(((pte) >> 48) & 0xFF)

/* Inner cache attrs MAIR_ATTR_N[3:0] */
#define NS_PTE_ATTR_INNER(pte)	((NS_PTE_ATTR_MAIR(pte)) & 0xF)

/* Outer cache attrs MAIR_ATTR_N[7:4] */
#define NS_PTE_ATTR_OUTER(pte)	(((NS_PTE_ATTR_MAIR(pte)) & 0xF0) >> 4)

#endif /* UTHREAD_WITH_MEMORY_MAPPING_SUPPORT */

typedef enum {
	PGTBL_NONE = 0,
	PGTBL_LEVEL_1_USER,
	PGTBL_LEVEL_1_PRIV,
	PGTBL_LEVEL_2,
} pgtbl_lvl_t;

status_t arm_uthread_mmu_map(uthread_t *ut, paddr_t paddr,
		vaddr_t vaddr, uint l1_flags, uint l2_flags);
status_t arm_uthread_mmu_unmap(uthread_t *ut, vaddr_t vaddr);

#endif
