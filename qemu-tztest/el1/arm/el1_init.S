#define __ASSEMBLY__
#include "platform.h"
#include "memory.h"
#include "cpu.h"
#include "vmsa.h"
#include "el1.h"
#undef __ASSEMBLY__

.arch_extension sec

.section .init
/* el1_init()  - r4 = syscntl PA */
el1_init:
    cps #CPSR_M_SVC

    /* Disable the MMU in case it is enabled.  This happens when we came from
     * aarch32 monitor mode.
     */
    mrc p15, 0, r10, c1, c0, 0
    and r10, r10, #~0x1
    mcr p15, 0, r10, c1, c0, 0

    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     /* Mask IRQ and FIQ */
    msr cpsr, r10

/* This may be a duplicate from EL3/monitor, but just in case */
init_uart:
	/* UART initialisation (38400 8N1) */
	ldr	r0,	=UART0_BASE     // UART base (Versatile Express)
	mov	r1, #0x10			// ibrd
	str	r1, [r0, #0x24]
	mov	r1, #0xc300
	orr	r1, #0x0001			// cr
	str	r1, [r0, #0x30]

    /* Set-up the el1 vbar to use low mem.  */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x2000           /* SCTLR.V = 0 */
    mcr p15, 0, r10, c1, c0, 0
    ldr r10, =el1_vectors
    mcr p15, 0, r10, c12, c0, 0     /* VBAR = el1_vectors */

    /* The stack still needs to be allocated and mapped so we set up a
     * temporary stack for the time being.
     */
    ldr r10, =EL1_INIT_STACK
	mov sp, r10

    /* Use the top of the stack to track our PA pool pointer */
    ldr r11, =EL1_PA_POOL_BASE
    ldr r10, =EL1_PTE_POOL_BASE
    push {r10, r11}

    /* Save the input syscntl buf pointer */
    push {r4}

    /* Enable floating-point as printf uses it */
    mrc p15, 0, r10, c1, c0, 2
    orr r10, r10, #CPACR_FPEN(0x3)
    mcr p15, 0, r10, c1, c0, 2

el1_mmu_init:
    /* Disable data and instruction caches */
    mrc p15, 0, r10, c1, c0, 0
    bic r10, r10, #0x0004
    bic r10, r10, #0x1000
    bic r10, r10, #0x20000000   /* Enable the simplified access model AFE=1 */
    mcr p15, 0, r10, c1, c0, 0

    /* Set domain 0 & 1 for client access */
    mov r10, #0x5
    mcr p15, 0, r10, c3, c0, 0

    /* Set TTBR0 to the initialized address plus enable shareable write-back
     * write-allocate.
     */
    ldr r10, =EL1_PGTBL_BASE
    orr r10, r10, #0xB
    mcr p15, 0, r10, c2, c0, 0

    /* Set-up the table base control to split between TTBR0/1 (N = 0) */
    mov r10, #0x0
    mcr p15, 0, r10, c2, c0, 2

el1_map_init:
    /* Direct map the init code */
    ldr r0, =_EL1_INIT_BASE
    mov r1, #PTE_PRIV_RO
    ldr r2, =_EL1_INIT_SIZE
    bl map_pa_range

el1_map_flash:
    /* Direct map the EL1 flash sections so we can copy from them once
     * the MMU has been enabled.
     */
    ldr r0, =_EL1_FLASH_TEXT
    mov r1, #PTE_PRIV_RO
    ldr r2, =_EL1_TEXT_SIZE
    bl map_pa_range

    ldr r0, =_EL1_FLASH_DATA
    mov r1, #PTE_PRIV_RO
    ldr r2, =_EL1_DATA_SIZE
    bl map_pa_range

el1_map_text:
    /* Map the EL1 text address range.  Initially this needs to be RW so we
     * can copy in the text from flash.
     */
    ldr r0, =_EL1_TEXT_BASE
    mov r1, #PTE_PRIV_RW
    ldr r2, =_EL1_TEXT_SIZE
    bl map_va_range

el1_map_data:
    /* Map the EL1 data address range. */
    ldr r0, =_EL1_DATA_BASE
    mov r1, #PTE_PRIV_RW
    ldr r2, =_EL1_DATA_SIZE
    bl map_va_range

el1_map_stack:
    /* Map the first page of the stack so we can get off the ground */
	ldr r0, =EL1_ABT_STACK_BASE
    sub r0, r0, #PAGE_SIZE
    mov r1, #PTE_PRIV_RW
    bl map_va

	ldr r0, =EL1_IRQ_STACK_BASE
    sub r0, r0, #PAGE_SIZE
    mov r1, #PTE_PRIV_RW
    bl map_va

	ldr r0, =EL1_UND_STACK_BASE
    sub r0, r0, #PAGE_SIZE
    mov r1, #PTE_PRIV_RW
    bl map_va

	ldr r0, =EL1_SVC_STACK_BASE
    sub r0, r0, #PAGE_SIZE
    mov r1, #PTE_PRIV_RW
    bl map_va

    ldr r0, =EL0_STACK_BASE
    sub r0, r0, #PAGE_SIZE
    mov r1, #PTE_USER_RW
    bl map_va

/* Map the PT page pool so we can access the PT after the MMU is enabled.
 * We only allcoate EL1_PGTBL_SIZE pages for PT use which consists of the
 * 2^(2-N) TTBR pages and the rest are PT page pool
 */
el1_map_pt:
    /* Direct map the page table pool */
    ldr r0, =EL1_PGTBL_BASE
    mov r1, #PTE_PRIV_RW
    ldr r2, =EL1_PGTBL_SIZE
    bl map_pa_range

el1_map_uart:
    ldr r0, =UART0_BASE
    mov r1, #PTE_USER_RW
    bl map_pa

el1_map_sysreg:
    ldr r0, =SYSREG_BASE;
    mov r1, #PTE_PRIV_RW
    bl map_pa

save_last_pa:
    pop {r4, r8, r9}

el1_mmu_enable:
    /* Enable the mmu */
    mcr p15, 0, r10, c8, c7, 0
    mrc p15, 0, r10, c1, c0, 0
    orr r10, r10, #0x1
    mcr p15, 0, r10, c1, c0, 0
    isb
    dsb

el1_init_stack:
    /* Set-up the el1 SVC stack */
	ldr sp, =EL1_SVC_STACK_BASE

    /* Migrate the next PA to the new stack we should not use it again until
     * after we transition to start.
     */
    push {r4, r8, r9}

    cps #CPSR_M_ABT
	ldr sp, =EL1_ABT_STACK_BASE

    cps #CPSR_M_IRQ
	ldr sp, =EL1_IRQ_STACK_BASE

    cps #CPSR_M_UND
	ldr sp, =EL1_UND_STACK_BASE

    /* Have to set user (and system) stack from SYS mode so we can get back to
     * SVC.
     */
    cps #CPSR_M_SYS
	ldr sp, =EL0_STACK_BASE

    cps #CPSR_M_SVC

/* The EL1 address space is set-up and the MMU is started so it is safe to copy
 * the text and data sections in.
 */
el1_copy_text:
    ldr r0, =_EL1_TEXT_BASE
    ldr r1, =_EL1_FLASH_TEXT
    ldr r2, =_EL1_TEXT_SIZE
    bl memcpy

el1_copy_data:
    ldr r0, =_EL1_DATA_BASE
    ldr r1, =_EL1_FLASH_DATA
    ldr r2, =_EL1_DATA_SIZE
    bl memcpy

el1_init_start:
    pop {r4, r8, r9}
    ldr r10, =syscntl
    str r4, [r10]
    ldr r10, =mem_next_pa
    str r9, [r10]
    ldr r10, =mem_next_l1_page
    str r8, [r10]

    ldr r0, =_EL1_INIT_BASE
    ldr r1, =_EL1_INIT_SIZE
    ldr r2, =el1_start
    bx r2

/* We should never get here */
el1_init_end:
    b el1_init_end

el1_remap_vecs:
    /* Now that we have enabled the MMU, move the mapping of the el1 vector
     * table to high mem.
     * No need to set up VBAR as we'll be using hivec */
    mrc p15, 0, r10, c1, c0, 0
    orr r10, r10, #0x2000           @ SCTLR.V = 1
    mcr p15, 0, r10, c1, c0, 0

#define INIT_STACK EL1_INIT_STACK
#include "init_util.S"
