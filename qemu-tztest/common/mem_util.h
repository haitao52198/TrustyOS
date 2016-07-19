#ifndef _MEM_UTIL_H
#define _MEM_UTIL_H

extern uintptr_t mem_allocate_pa();
extern void mem_map_pa(uintptr_t, uintptr_t, uintptr_t, uintptr_t);
extern void mem_map_va(uintptr_t, uintptr_t, uintptr_t);
extern int mem_unmap_va(uintptr_t addr);
extern void *mem_heap_allocate(size_t len);
extern void *mem_lookup_pa(void *va);

#endif
