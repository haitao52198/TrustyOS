/*
 * Copyright (c) 2015 Google Inc. All rights reserved
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
#include <assert.h>
#include <trace.h>
#include <debug.h>
#include <arch/mmu.h>
#include <lib/sm.h>

#define LOCAL_TRACE 0

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
#define NS_PTE_AP_U(pte)		(NS_PTE_AP(pte) & 0x1)
#define NS_PTE_AP_RO(pte)		(NS_PTE_AP(pte) & 0x2)

/* Shareablility attrs */
#define NS_PTE_ATTR_SHAREABLE(pte)	(((pte) >> 8) & 0x3)

/* cache attrs encoded in the top bits 55:49 of the PTE*/
#define NS_PTE_ATTR_MAIR(pte)		(((pte) >> 48) & 0xFF)

/* Inner cache attrs MAIR_ATTR_N[3:0] */
#define NS_PTE_ATTR_INNER(pte)		((NS_PTE_ATTR_MAIR(pte)) & 0xF)

/* Outer cache attrs MAIR_ATTR_N[7:4] */
#define NS_PTE_ATTR_OUTER(pte)		(((NS_PTE_ATTR_MAIR(pte)) & 0xF0) >> 4)

/* Normal memory */
#define NS_MAIR_NORMAL_CACHED_WB_RWA       0xFF /* inner and outer write back read/write allocate */
#define NS_MAIR_NORMAL_CACHED_WT_RA        0xAA /* inner and outer write through read allocate */
#define NS_MAIR_NORMAL_CACHED_WB_RA        0xEE /* inner and outer wriet back, read allocate */
#define NS_MAIR_NORMAL_UNCACHED            0x44 /* uncached */

/* Device memory */
#define NS_MAIR_DEVICE_STRONGLY_ORDERED    0x00 /* nGnRnE (strongly ordered) */
#define NS_MAIR_DEVICE                     0x04 /* nGnRE  (device) */
#define NS_MAIR_DEVICE_GRE                 0x0C /* GRE */

/* sharaeble attributes */
#define NS_NON_SHAREABLE                   0x0
#define NS_OUTER_SHAREABLE                 0x2
#define NS_INNER_SHAREABLE                 0x3

/* helper function to decode ns memory attrubutes  */
status_t sm_decode_ns_memory_attr(struct ns_page_info *pinf,
				  ns_addr_t *ppa, uint *pmmu)
{
	uint mmu_flags = 0;

	if(!pinf)
		return ERR_INVALID_ARGS;

	LTRACEF("raw=0x%llx: pa=0x%llx: mair=0x%x, sharable=0x%x\n",
		pinf->attr,
		NS_PTE_PHYSADDR(pinf->attr),
		(uint)NS_PTE_ATTR_MAIR(pinf->attr),
		(uint)NS_PTE_ATTR_SHAREABLE(pinf->attr));

	if (ppa)
		*ppa = (ns_addr_t)NS_PTE_PHYSADDR(pinf->attr);

	if (pmmu) {
		/* match settings to mmu flags */
		switch ((uint)NS_PTE_ATTR_MAIR(pinf->attr)) {
			case NS_MAIR_NORMAL_CACHED_WB_RWA:
				mmu_flags |= ARCH_MMU_FLAG_CACHED;
				break;
			case NS_MAIR_NORMAL_UNCACHED:
				mmu_flags |= ARCH_MMU_FLAG_UNCACHED;
				break;
			default:
				LTRACEF("Unsupported memory attr 0x%x\n",
				        (uint)NS_PTE_ATTR_MAIR(pinf->attr));
				return ERR_NOT_SUPPORTED;
		}
#if WITH_SMP
		if (mmu_flags == ARCH_MMU_FLAG_CACHED) {
			if(NS_PTE_ATTR_SHAREABLE(pinf->attr) != NS_INNER_SHAREABLE) {
				LTRACEF("Unsupported sharable attr 0x%x\n",
					(uint)NS_PTE_ATTR_SHAREABLE(pinf->attr));
				return ERR_NOT_SUPPORTED;
			}
		}
#endif
		if (NS_PTE_AP_U(pinf->attr))
			mmu_flags |= ARCH_MMU_FLAG_PERM_USER;

		if (NS_PTE_AP_RO(pinf->attr))
			mmu_flags |= ARCH_MMU_FLAG_PERM_RO;

		*pmmu = mmu_flags | ARCH_MMU_FLAG_NS | ARCH_MMU_FLAG_PERM_NO_EXECUTE;
	}

	return NO_ERROR;
}


/* Helper function to get NS memory buffer info out of smc32 call params */
status_t smc32_decode_mem_buf_info(struct smc32_args *args, ns_addr_t *ppa,
                                   ns_size_t *psz, uint *pmmu)
{
	status_t res;
	struct ns_page_info pi;

	DEBUG_ASSERT(args);

	pi.attr = ((uint64_t)args->params[1] << 32) | args->params[0];

	res = sm_decode_ns_memory_attr(&pi, ppa, pmmu);
	if (res != NO_ERROR)
		return res;

	if (psz)
		*psz = (ns_size_t)args->params[2];

	return NO_ERROR;
}

