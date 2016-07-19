/* allocate_pa() - Allocates and returns next pool PA */
allocate_pa:
    push {r10, r11, lr}
    ldr r10, =INIT_STACK
    ldr r0, [r10, #-4]
    add r11, r0, #0x1000
    str r11, [r10, #-4]
    pop {r10, r11, lr}
    bx lr

allocate_l1_page:
    push {r10, r11, lr}
    ldr r10, =INIT_STACK
    ldr r0, [r10, #-8]
    add r11, r0, #0x1000
    str r11, [r10, #-8]
    pop {r10, r11, lr}
    bx lr

.globl map_va_to_pa
/* map_va_to_pa(VA, pgprop, PA) */
map_va_to_pa:
    push {r7-r12, lr}
    mrc p15, 0, r7, c2, c0, 0
    mov r10, #-1                    /* Creating entry mask */
    lsl r9, r10, #14                /* TTBCR.N = 0, so base starts at bit 14 */
    and r7, r7, r9                  /* TTBR translation base */
    lsr r8, r0, #20                 /* Shift VA to get L1 index */
    movw r10, #0xFFF                /* TTBCR.N = 0, so L1 idx mask is 12 bits */
    and r8, r8, r10                 /* Mask off L1 index from VA */
    lsl r8, r8, #2                  /* Shift into place for the PTE */
    orr r8, r8, r7                  /* L1 descriptor address */
    ldr r7, [r8]                    /* L1 descriptor */
    and r9, r7, #0x3                /* Filter valid bits */
    cmp r9, #0                      /* 0 is invalid */
    bne map_l2                      /* Skip to L2 if already valid */
    mov r9, r0                      /* Save the input VA */
    bl allocate_l1_page             /* Allocate a PT phys page */
    mov r7, r0                      /* Use PA to start PT entry */
    mov r0, r9                      /* Restore VA */
    mov r9, #0                      /* Page table page properties TBD */
    orr r7, r7, r9                  /* Add page table page properties */
    orr r7, r7, #0x1                /* Page table page */
    str r7, [r8]                    /* Save the PTE to the L1 location */
map_l2:
    mov r10, #-1                    /* Creating entry mask */
    lsl r9, r10, #10                /* L2 base starts at bit 10 */
    and r7, r7, r9                  /* TTBR translation base */
    lsr r8, r0, #12                 /* Shift VA to get L2 index */
    and r8, r8, #0xFF               /* Mask index based on TTBCR.N = 0 */
    lsl r8, r8, #2                  /* Shift into place for the PTE */
    orr r8, r8, r7                  /* L2 descriptor address */
    ldr r7, [r8]                    /* L2 descriptor */
    and r9, r7, #0x3                /* Filter valid bits */
    cmp r9, #0                      /* 0 is invalid */
    bne map_done                    /* Out if already valid */
    mov r7, r2                      /* Use PA to start PT entry */
    orr r7, r7, r1                  /* Add page table page properties */
    orr r7, r7, #0x2                /* Small page entry */
    str r7, [r8]                    /* Save the PTE to the L2 location */
map_done:
    pop {r7-r12, lr}
    bx lr

.globl unmap_va
/* unmap_va(VA) */
unmap_va:
    push {r7-r12, lr}
    ldr r10, =EL3_PGTBL_BASE
    mov r11, #0x4
    mov r12, #39
unmap_loop:
    movw r9, 0xF000
    movt r9, 0xFFFF
    and r9, r9, r10           /* Strip off descriptor non-address bits */
    lsr r8, r0, r12                /* Shift out VA bits for the level */
    sub r12, r12, #9                /* Update shift amount for next level */
    movw r7, #0x1FF
    and r8, r8, r7            /* Filter top VA bits for PT offset */
    lsl r8, r8, #3                /* Shift PT offset to bytes */
    orr r8, r8, r9               /* Compute descriptor address */
    sub r11, r11, #1                /* Decrease level */
    cmp r11, #0
    beq unmap_page             /* If we reached level 0 then finalize */
    ldr r10, [r8]                  /* Otherwise, fetch the descriptor */
    and r9, r10, #0x1              /* Filter valid bit */
    cmp r9, #0
    beq unmap_done             /* Assume an invalid PT page means done */
    b unmap_loop                    /* Next level */
unmap_page:
    mov r10, #0                     /* Clear the page PTE */
    str r10, [r8]                  /* Fill in PT entry */
unmap_done:
    pop {r7-r12, lr}
    bx lr

.globl map_va
/* map_va(VA, pgprop) */
map_va:
    push {r2, r10, lr}
    mov r10, r0
    bl allocate_pa
    mov r2, r0
    mov r0, r10
    bl map_va_to_pa
    pop {r2, r10, lr}
    bx lr

.globl map_pa
/* map_pa(PA, pgprop) */
map_pa:
    push {r2, lr}
    mov r2, r0
    bl map_va_to_pa
    pop {r2, lr}
    bx lr

.globl map_va_to_pa_range
/* map_va_to_pa_range(VA, pgprop, PA, len) */
map_va_to_pa_range:
    push {r0, r2, r3, r8, lr}
    movw r8, #0xFFF
    add r3, r3, r8
    movw r8, #0xF000
    movt r8, #0xFFFF
    and r3, r3, r8
map_va_to_pa_loop:
    cmp r3, #0
    beq map_va_to_pa_done
    bl map_va_to_pa
    add r0, r0, #0x1000
    add r2, r2, #0x1000
    sub r3, r3, #0x1000
    b map_va_to_pa_loop
map_va_to_pa_done:
    pop {r0, r2, r3, r8, lr}
    bx lr

/* map_pa_range(PA, pgprop, len) */
map_pa_range:
    push {r0, r2, r8, lr}
    movw r8, #0xFFF
    add r2, r2, r8
    movw r8, #0xF000
    movt r8, #0xFFFF
    and r2, r2, r8
map_pa_loop:
    cmp r2, #0
    beq map_pa_done
    bl map_pa
    add r0, r0, #0x1000
    sub r2, r2, #0x1000
    b map_pa_loop
map_pa_done:
    pop {r0, r2, r8, lr}
    bx lr

/* map_va_range(VA, pgprop, len) */
map_va_range:
    push {r0, r2, r8, lr}
    movw r8, #0xFFF
    add r2, r2, r8
    movw r8, #0xF000
    movt r8, #0xFFFF
    and r2, r2, r8
map_va_loop:
    cmp r2, #0
    beq map_va_done
    bl map_va
    add r0, r0, #0x1000
    sub r2, r2, #0x1000
    b map_va_loop
map_va_done:
    pop {r0, r2, r8, lr}
    bx lr

/* unmap_va_range(VA, len) */
unmap_va_range:
    push {r0, r1, r8, lr}
    movw r8, #0xFFF
    add r1, r1, r8
    movw r8, #0xF000
    movt r8, #0xFFFF
    and r1, r1, r8
unmap_va_loop:
    cmp r1, #0
    beq unmap_va_done
    bl unmap_va
    add r0, r0, #0x1000
    sub r1, r1, #0x1000
    b unmap_va_loop
unmap_va_done:
    pop {r0, r1, r8, lr}
    bx lr

/* memcpy(dest, src) */
memcpy:
    cmp r2, #0
    beq memcpy_done
    ldr r10, [r1], #4
    str r10, [r0], #4
    subs r2, r2, #4
    b memcpy
memcpy_done:
    bx lr
