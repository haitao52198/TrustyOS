#include "platform.h"
#include "memory.h"
#include "cpu.h"
#include "vmsa.h"

#define PT_BASE EL3_PGTBL_BASE

.section .init
.align 12
el3_init:
init_uart:
	/* UART initialisation (38400 8N1) */
	ldr	x0,	=UART0_BASE     // UART base
	mov	x1, #0x10			// ibrd
	str	x1, [x0, #0x24]
	mov	x1, #0xc300
	orr	x1, x1, #0x0001	    // cr
	str	x1, [x0, #0x30]

    /* Set-up the EL3 vbar */
    ldr x10, =el3_vectors
    msr vbar_el3, x10

    /* The stack still needs to be allocated and mapped so we set up a
     * temporary stack for the time being.
     */
    ldr x10, =RAM_BASE + (2 * PAGE_SIZE)
	mov sp, x10

    /* Use the top of the stack to track our PA pool pointer */
    ldr x10, =PT_BASE + (PAGE_SIZE)
    str x10, [sp]

    /* Enable floating point register usage as printf uses it */
    mrs x10, cptr_el3
    and x10, x10, #~CPTR_TFP       /* Clear CPTR.TFP to enable FP */
    msr cptr_el3, x10
    mrs x10, cpacr_el1
    orr x10, x10, #CPACR_FPEN(0x3)
    msr cpacr_el1, x10

el3_init_mmu:
    /* Disable data and instruction caches */
    mrs x10, sctlr_el3
    bic x10, x10, #0x0004
    bic x10, x10, #0x1000
    msr sctlr_el3, x10

    ldr x10, =PT_BASE           /* Base of L0 page table */
    lsr x11, x10, #TnSZ         /* Shift the invalid bits out */
    lsl x11, x11, #TnSZ
    msr ttbr0_el3, x11
    mov x11, #TnSZ
    msr tcr_el3, x11            /* PS = 32bit, TG0 = 4k, TnSZ */

el3_map_init:
    /* Direct map the init code */
    ldr x0, =_EL3_INIT_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RO)
    ldr x2, =_EL3_INIT_SIZE
    bl map_pa_range

el3_map_flash:
    /* Direct map the EL3 flash sections so we can copy from them once
     * the MMU has been enabled.
     */
    ldr x0, =_EL3_FLASH_TEXT
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RO)
    ldr x2, =_EL3_TEXT_SIZE
    bl map_pa_range

    ldr x0, =_EL3_FLASH_DATA
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RO)
    ldr x2, =_EL3_DATA_SIZE
    bl map_pa_range

el3_map_text:
    /* Map the EL3 text address range.  Initially this needs to be RW so we can
     * copy in the text from flash.
     */
    ldr x0, =_EL3_TEXT_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    ldr x2, =_EL3_TEXT_SIZE
    bl map_va_range

el3_map_data:
    /* Map the EL3 data address range. */
    ldr x0, =_EL3_DATA_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    ldr x2, =_EL3_DATA_SIZE
    bl map_va_range

el3_map_stack:
    /* Map the first page of the stack so we can get off the ground */
    ldr x0, =EL3_STACK_BASE - PAGE_SIZE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    bl map_va

el3_map_pt:
    /* Direct map the page table pool */
    ldr x0, =EL3_PGTBL_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    ldr x2, =EL3_PGTBL_SIZE
    bl map_pa_range

el3_map_uart:
    ldr x0, =UART0_BASE
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    bl map_pa

el3_map_sysreg:
    ldr x0, =SYSREG_BASE;
    mov x1, #(PTE_PAGE|PTE_ACCESS|PTE_PRIV_RW)
    bl map_pa

save_last_pa:
    ldr x17, [sp]

/* We should have all the critical address regions mapped at this point.
 * Anything that has not already been mapped will be handled on demand.  If it
 * needs to be copied out of flash, it needs to be mapped.
 * The mapped regions are:
 *  - EL3 init code - So we can keep executing after the MMU is enabled
 *  - EL3 flash region - So we can copy the code and data
 *  - EL3 code and text segments - Need to be copied from flash
 *  - EL3 initial stack page - Needed by the exception handler
 *  - EL3 PT PA pages - Needed so we don't fault on demand paging
 *  - UART - So we can use it to print
 */
el3_enable_mmu:
    mrs x10, sctlr_el3
    orr x10, x10, #0x1      // Enable MMU
    msr sctlr_el3, x10
    isb
    dsb sy

/* Now that the MMU is enabled and the inital stack page is mapped we can
 * safely set the stack pointer.
 */
el3_init_stack:
    ldr x10, =EL3_STACK_BASE
    mov sp, x10

/* Migrate the next PA to the new stack we should not use it again until after
 * we transition to start.
 */
    str x17, [sp, #-8]!

/* The EL3 address space is set-up and the MMU is started so it is safe to copy
 * the text and data sections in.
 */
el3_copy_text:
    ldr x0, =_EL3_TEXT_BASE
    ldr x1, =_EL3_FLASH_TEXT
    ldr x2, =_EL3_TEXT_SIZE
    bl memcpy

el3_copy_data:
    ldr x0, =_EL3_DATA_BASE
    ldr x1, =_EL3_FLASH_DATA
    ldr x2, =_EL3_DATA_SIZE
    bl memcpy

el3_init_start:
    ldr x17, [sp], #8       /* next PA */
    ldr x10, =mem_next_pa
    str x17, [x10]

    ldr x0, =_EL3_INIT_BASE
    ldr x1, =_EL3_INIT_SIZE
    ldr x2, =el3_start
    br x2

/* We should never get here */
el3_init_end:
    b el3_init_end

#include "init_util.S"

.end
