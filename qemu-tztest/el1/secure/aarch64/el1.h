#ifndef _EL1_S_H
#define _EL1_S_H

#include "memory.h"

#ifndef __ASSEMBLY__
extern uintptr_t _EL1_S_INIT_BASE;
extern uintptr_t EL1_S_INIT_BASE;
extern uintptr_t _EL1_S_INIT_SIZE;
extern uintptr_t EL1_S_INIT_SIZE;
extern uintptr_t _EL1_S_FLASH_TEXT;
extern uintptr_t EL1_S_FLASH_TEXT;
extern uintptr_t _EL1_S_TEXT_BASE;
extern uintptr_t EL1_S_TEXT_BASE;
extern uintptr_t _EL1_S_DATA_BASE;
extern uintptr_t EL1_S_DATA_BASE;
extern uintptr_t _EL1_S_TEXT_SIZE;
extern uintptr_t EL1_S_TEXT_SIZE;
extern uintptr_t _EL1_S_DATA_SIZE;
extern uintptr_t EL1_S_DATA_SIZE;
#endif

#define _EL1_INIT_BASE _EL1_S_INIT_BASE
#define _EL1_INIT_SIZE _EL1_S_INIT_SIZE
#define _EL1_FLASH_TEXT _EL1_S_FLASH_TEXT
#define _EL1_TEXT_BASE _EL1_S_TEXT_BASE
#define _EL1_TEXT_SIZE _EL1_S_TEXT_SIZE
#define _EL1_FLASH_DATA _EL1_S_FLASH_DATA
#define _EL1_DATA_BASE _EL1_S_DATA_BASE
#define _EL1_DATA_SIZE _EL1_S_DATA_SIZE
#define EL1_STACK_BASE EL1_S_STACK_BASE
#define EL1_PGTBL_BASE EL1_S_PGTBL_BASE
#define EL1_PGTBL_SIZE EL1_S_PGTBL_SIZE
#define EL1_INIT_STACK EL1_S_INIT_STACK
#define EL1_PA_POOL_BASE EL1_S_PA_POOL_BASE
#define EL1_PTE_POOL_BASE EL1_S_PTE_POOL_BASE
#define EL1_BASE_VA EL1_S_BASE_VA
#define EL1_VA_HEAP_BASE EL1_S_VA_HEAP_BASE

#define EL0_STACK_BASE EL0_S_STACK_BASE

#endif
