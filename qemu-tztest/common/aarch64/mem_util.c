#include "libcflat.h"
#include <stdint.h>
#include "vmsa.h"
#include <stddef.h>
#include "mem_util.h"

#define SEC_STATE_STR "EL3"
#include "debug.h"

extern uintptr_t mem_pgtbl_base;
extern uintptr_t mem_next_pa;
extern uintptr_t mem_next_l1_page;
extern uintptr_t mem_heap_pool;

uintptr_t mem_allocate_pa()
{
    uintptr_t next = mem_next_pa;
    mem_next_pa += PAGE_SIZE;
    return next;
}

void mem_map_pa(uintptr_t vaddr, uintptr_t paddr,
                uintptr_t tblattr, uintptr_t pgattr)
{
    uintptr_t pa = mem_pgtbl_base, off;
    uint32_t i;
    uintptr_t *pte;

    for (i = 0; i < 3; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        off = ((vaddr >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uintptr_t *)(pa | off);
        if (!(*pte & 0x1)) {
            pa = mem_allocate_pa();
        	mem_map_pa(pa, pa, 0, PTE_USER_RW);
            *pte = pa;
            *pte |= (PTE_TABLE | tblattr);
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    /* The last level is the physical page to map */
    off = ((vaddr >> (39-(i*9))) & 0x1FF) << 3;
    pte = (uintptr_t *)(pa | off);
    *pte = paddr & ~0xFFF;
    *pte |= (PTE_PAGE | PTE_ACCESS | pgattr);
    DEBUG_MSG("mapped VA:0x%lx to PA:0x%x - PTE:0x%lx (0x%lx)",
              vaddr, paddr, pte, *pte);
}

void mem_map_va(uintptr_t addr, uintptr_t tblattr, uintptr_t pgattr)
{
    uintptr_t pa = mem_allocate_pa();

    mem_map_pa(addr, pa, tblattr, pgattr);
}

int mem_unmap_va(uintptr_t addr)
{
    uintptr_t pa = mem_pgtbl_base;
    uint32_t i;
    uintptr_t *pte;

    for (i = 0; i < 4; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        uintptr_t off = ((addr >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uintptr_t *)(pa | off);
        if (!(*pte & 0x1)) {
            DEBUG_MSG("Failed unmap: invalid table page");
            /* This is not a valid page, return an error */
            return -1;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    /* Clear the page descriptor */
    *pte = 0;
    DEBUG_MSG("unmapped PTE 0x%lx (VA:0x%lx, PA:0x%x)",
              pte, addr, pa);

    return 0;
}

void *mem_heap_allocate(size_t len)
{
    void *addr = (void *)mem_heap_pool;
    size_t off;

    for (off = 0; off < len; off += 0x1000) {
        mem_map_va(mem_heap_pool + off, 0, PTE_USER_RW);
    }

    mem_heap_pool += off;

    return addr;
}

void *mem_lookup_pa(void *va)
{
    uintptr_t pa = mem_pgtbl_base;
    uint32_t i;
    uintptr_t *pte;

    for (i = 0; i < 4; i++) {
        /* Each successive level uses the next lower 9 VA bits in a 48-bit
         * address, hence the i*9.
         */
        uintptr_t off = ((((uint64_t)va) >> (39-(i*9))) & 0x1FF) << 3;
        pte = (uintptr_t *)(pa | off);
        if (!(*pte & 0x1)) {
            DEBUG_MSG("Failed Lookup: invalid table page");
            /* This is not a valid page, return an error */
            return (void *)-1;
        } else {
            pa = *pte & 0x000FFFFFF000;
        }
    }

    return (void *)pa;
}

