#define __ASSEMBLY__
#include "memory.h"
#include "vmsa.h"
#include "cpu.h"
#include "el1.h"
#undef __ASSEMBLY__

.section .init
.align 12
.global el1_init
/* el1_init()  - x4 = syscntl PA */
el1_init:
    /* Set-up the EL1 vbar */
    ldr x10, =el1_vectors
    msr vbar_el1, x10

    /* The stack still needs to be allocated and mapped so we set up a
     * temporary stack for the time being.
     */
    ldr x10, =RAM_BASE + (2 * PAGE_SIZE)
	mov sp, x10

    /* Use the top of the stack to track our PA pool pointer */
    ldr x10, =EL1_PGTBL_BASE + PAGE_SIZE
    str x10, [sp]

    /* Save the input syscntl buf pointer */
    str x4, [sp, #-8]!

    /* Enable floating point register usage as printf uses it */
    mrs x10, cpacr_el1
    orr x10, x10, #CPACR_FPEN(0x3)
    msr cpacr_el1, x10

el1_init_mmu:
    /* Disable data and instruction caches */
    mrs x10, sctlr_el1
    bic x10, x10, #0x0004
    bic x10, x10, #0x1000
    msr sctlr_el1, x10

    ldr x10, =EL1_PGTBL_BASE    /* Base of L0 page table */
    lsr x11, x10, #TnSZ         /* Shift the invalid bits out */
    lsl x11, x11, #TnSZ
    msr ttbr0_el1, x11
    mov x11, #TnSZ
    msr tcr_el1, x11            /* PS = 32bit, TG0 = 4k, TnSZ */

el1_map_init:
    /* Direct map the init code */
    ldr x0, =_EL1_INIT_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_USER_RO)
    ldr x2, =_EL1_INIT_SIZE
    bl map_pa_range

el1_map_flash:
    /* Direct map the EL1 flash sections so we can copy from them once
     * the MMU has been enabled.
     */
    ldr x0, =_EL1_FLASH_TEXT
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_USER_RO)
    ldr x2, =_EL1_TEXT_SIZE
    bl map_pa_range

    ldr x0, =_EL1_FLASH_DATA
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_USER_RW)
    ldr x2, =_EL1_DATA_SIZE
    bl map_pa_range

el1_map_text:
    /* Map the EL1 text address range */
    ldr x0, =_EL1_TEXT_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    ldr x2, =_EL1_TEXT_SIZE
    bl map_va_range

el1_map_data:
    /* Map the EL1 data address range */
    ldr x0, =_EL1_DATA_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    ldr x2, =_EL1_DATA_SIZE
    bl map_va_range

el1_map_stacks:
    /* Map the first page of the stack so we can get off the ground */
    ldr x0, =EL1_STACK_BASE - PAGE_SIZE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    bl map_va

    ldr x0, =EL0_STACK_BASE - PAGE_SIZE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_USER_RW)
    bl map_va

el1_map_pt:
    /* Direct map the page table pool */
    ldr x0, =EL1_PGTBL_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    ldr x2, =EL1_PGTBL_SIZE
    bl map_pa_range

el1_map_uart:
    ldr x0, =UART0_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_USER_RW)
    bl map_pa

/* Save the next PA pointer until after we have enabled the MMU */
save_stack:
    ldr x16, [sp], #8       /* init input - smc_interop */
    ldr x17, [sp]           /* next PA */

/* We should have all the critical address regions mapped at this point.
 * Anything that has not already been mapped will be handled on demand.  If it
 * needs to be copied out of flash, it needs to be mapped.
 * The mapped regions are:
 *  - EL1 init code - So we can keep executing after the MMU is enabled
 *  - EL1 flash region - So we can copy the code and data
 *  - EL1 code and text segments - Need to be copied from flash
 *  - EL1 initial stack page - Needed by the exception handler
 *  - EL1 PT PA pages - Needed so we don't fault on demand paging
 *  - UART - So we can use it to print
 */
el1_enable_mmu:
    mrs x10, sctlr_el1
    orr x10, x10, #0x1      // Enable MMU
    msr sctlr_el1, x10
    isb
    dsb sy

/* Now that the MMU is enabled and the inital stack page is mapped we can
 * safely set the stack pointer.  The stacks are started at on quadword below
 * the stack base so two pages don't need to be mapped.
 */
el1_init_stack:
    ldr x10, =EL1_STACK_BASE-0x10
    mov sp, x10
    stp x16, x17, [sp, #-16]!

el0_init_stack:
    ldr x10, =EL0_STACK_BASE-0x10
    msr sp_el0, x10

/* The EL1 address space is set-up and the MMU is started so it is safe to copy
 * the text and data sections in and unmap the respective flash.
 */
el1_copy_text:
    ldr x0, =_EL1_TEXT_BASE
    ldr x1, =_EL1_FLASH_TEXT
    ldr x2, =_EL1_TEXT_SIZE
    bl memcpy

    ldr x0, =_EL1_FLASH_TEXT
    ldr x1, =_EL1_TEXT_SIZE
    bl unmap_va_range

el1_copy_data:
    ldr x0, =_EL1_DATA_BASE
    ldr x1, =_EL1_FLASH_DATA
    ldr x2, =_EL1_DATA_SIZE
    bl memcpy

    ldr x0, =_EL1_FLASH_DATA
    ldr x1, =_EL1_DATA_SIZE
    bl unmap_va_range

el1_init_start:
    ldp x16, x17, [sp], #16

/* Migrate the next PA to the non-init code */
    ldr x10, =mem_next_pa
    str x17, [x10]
    ldr x10, =syscntl
    str x16, [x10]

    /* Pass the address and size of the init section to start so it
     * can unmap it.  We must wait until we are in start because we are still
     * running out of the init segment.
     */
    ldr x0, =_EL1_INIT_BASE
    ldr x1, =_EL1_INIT_SIZE
    b el1_start

/* We should never get here */
el1_init_end:
    b el1_init_end

#define PT_BASE EL1_PGTBL_BASE      /* Set the PT base before including */
#include "init_util.S"
.end
