#define __ASSEMBLY__
#include "memory.h"
#include "cpu.h"
#include "vmsa.h"
#include "platform.h"
#undef __ASSEMBLY__

.arch_extension sec

#define INIT_STACK EL3_INIT_STACK

.section .init
monitor_init:
    cps #CPSR_M_MON

init_uart:
	/* UART initialisation (38400 8N1) */
	ldr	r0,	=UART0_BASE     // UART base (Versatile Express)
	mov	r1, #0x10			// ibrd
	str	r1, [r0, #0x24]
	mov	r1, #0xc300
	orr	r1, #0x0001			// cr
	str	r1, [r0, #0x30]

    /* Disable interrupts for now */
    mrs r10, cpsr
    orr r10, r10, #0xc0     /* Mask IRQ and FIQ */
    msr cpsr, r10

    /* Set-up the monitor vbar
     * During startup, before the MMU is enabled we'll set-up the vecs in low
     * mem.
     */
    ldr r10, =el3_vectors
    mcr p15, 0, r10, c12, c0, 1         /* MVBAR */

    /* The stack still needs to be allocated and mapped so we set up a
     * temporary stack for the time being.
     */
    ldr r10, =INIT_STACK
	mov sp, r10

    /* Use the top of the stack to track our PA pool pointer */
    ldr r11, =EL3_PA_POOL_BASE
    ldr r10, =EL3_PTE_POOL_BASE
    push {r10, r11}

    /* Enable floating-point as printf uses it */
    mrc p15, 0, r10, c1 , c0, 2
    orr r10, r10, #CPACR_FPEN(0x3)
    mcr p15, 0, r10, c1 , c0, 2
    mrc p15, 0, r10, c1, c1, 2
    orr r10, r10, #NSACR_CP10
    mcr p15, 0, r10, c1, c1, 2

monitor_mmu_init:
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
    ldr r10, =EL3_PGTBL_BASE
    orr r10, r10, #0xB
    mcr p15, 0, r10, c2, c0, 0

    /* Set-up the table base control to split between TTBR0/1 (N = 0) */
    mov r10, #0x0
    mcr p15, 0, r10, c2, c0, 2

    /* Clear the page table pages */
    ldr r10, =EL3_PGTBL_BASE
    ldr r11, =EL3_PGTBL_SIZE
    mov r0, #0
monitor_pt_clear:
    cmp r11, #0
    beq monitor_map_init
    str r0, [r10], #4
    sub r11, r11, #4
    b monitor_pt_clear

monitor_map_init:
    /* Direct map the init code */
    ldr r0, =_EL3_INIT_BASE
    movw r1, #PTE_PRIV_RO
    ldr r2, =_EL3_INIT_SIZE
    bl map_pa_range

monitor_map_flash:
    /* Direct map the EL3 flash sections so we can copy from them once
     * the MMU has been enabled.
     */
    ldr r0, =_EL3_FLASH_TEXT
    mov r1, #PTE_PRIV_RO
    ldr r2, =_EL3_TEXT_SIZE
    bl map_pa_range

    ldr r0, =_EL3_FLASH_DATA
    mov r1, #PTE_PRIV_RO
    ldr r2, =_EL3_DATA_SIZE
    bl map_pa_range

monitor_map_text:
    /* Map the EL3 text address range.  Initially this needs to be RW so we can
     * copy in the text from flash.
     */
    ldr r0, =_EL3_TEXT_BASE
    mov r1, #PTE_PRIV_RW
    ldr r2, =_EL3_TEXT_SIZE
    bl map_va_range

monitor_map_data:
    /* Map the EL3 data address range. */
    ldr r0, =_EL3_DATA_BASE
    mov r1, #PTE_PRIV_RW
    ldr r2, =_EL3_DATA_SIZE
    bl map_va_range

monitor_map_stack:
    /* Map the first page of the stack so we can get off the ground */
    ldr r0, =0x7FFEF000   // EL3_STACK_BASE-0x1000
    mov r1, #PTE_PRIV_RW
    bl map_va

/* Map the PT page pool so we can access the PT after the MMU is enabled.
 * We only allcoate EL3_PGTBL_SIZE pages for PT use which consists of the
 * 2^(2-N) TTBR pages and the rest are PT page pool
 */
monitor_map_pt:
    /* Direct map the page table pool */
    ldr r0, =EL3_PGTBL_BASE
    mov r1, #PTE_PRIV_RW
    ldr r2, =0x4000
    bl map_pa_range

    ldr r0, =EL3_PTE_POOL_BASE
    mov r1, #PTE_PRIV_RW
    ldr r2, =0x3C000
    bl map_pa_range

monitor_map_uart:
    ldr r0, =UART0_BASE
    mov r1, #PTE_USER_RW
    bl map_pa

monitor_map_sysreg:
    ldr r0, =SYSREG_BASE;
    mov r1, #PTE_PRIV_RW
    bl map_pa

/* We need to map the first page of secure EL1 as we will be doing an exception
 * return to it in order to initialize it.  We only map the first page as this
 * is enough to get it off the ground.
 */
monitor_map_el1:
    /* Direct map the init code */
    ldr r0, =EL1_S_FLASH_BASE
    mov r1, #PTE_PRIV_RO
    bl map_pa

save_last_pa:
    pop {r8, r9}

monitor_mmu_enable:
    /* Enable the mmu */
    mcr p15, 0, r10, c8, c7, 0
    mrc p15, 0, r10, c1, c0, 0
    orr r10, r10, #0x1
    mcr p15, 0, r10, c1, c0, 0
    isb
    dsb

monitor_init_stack:
    /* Set-up the monitor SVC stack */
	ldr sp, =EL3_STACK_BASE

    /* Migrate the next PA to the new stack we should not use it again until
     * after we transition to start.
     */
    push {r8, r9}

/* The EL3 address space is set-up and the MMU is started so it is safe to copy
 * the text and data sections in.
 */
monitor_copy_text:
    ldr r0, =_EL3_TEXT_BASE
    ldr r1, =_EL3_FLASH_TEXT
    ldr r2, =_EL3_TEXT_SIZE
    bl memcpy

monitor_copy_data:
    ldr r0, =_EL3_DATA_BASE
    ldr r1, =_EL3_FLASH_DATA
    ldr r2, =_EL3_DATA_SIZE
    bl memcpy

monitor_init_start:
    pop {r8, r9}
    ldr r10, =mem_next_pa
    str r9, [r10]
    ldr r10, =mem_next_l1_page
    str r8, [r10]

    ldr r0, =_EL3_INIT_BASE
    ldr r1, =_EL3_INIT_SIZE
    ldr r2, =el3_start
    blx r2

/* We should never get here */
monitor_init_end:
    b monitor_init_end

#include "init_util.S"
