#include "vmsa.h"

.section .init
/* allocate_pa() - Allocates and returns next pool PA */
allocate_pa:
    stp x10, x11, [sp, #-16]!
    str x30, [sp, #-8]!
    ldr x10, =RAM_BASE+0x2000
    ldr x0, [x10]
    add x11, x0, #0x1000
    str x11, [x10]
    ldr x30, [sp], #8
    ldp x10, x11, [sp], #16
    ret

.globl map_va_to_pa
/* map_va_to_pa(VA, pgprop, PA) */
map_va_to_pa:
    stp x30, x10, [sp, #-16]!
    stp x11, x12, [sp, #-16]!
    stp x13, x14, [sp, #-16]!
    ldr x12, =PT_BASE
    mov x13, #0x4
    mov x14, #39
map_loop:
    and x11, x12, #~0xFFF           /* Strip off descriptor non-address bits */
    lsr x10, x0, x14                /* Shift out VA bits for the level */
    sub x14, x14, #9                /* Update shift amount for next level */
    and x10, x10, #0x1FF            /* Filter top VA bits for PT offset */
    lsl x10, x10, #3                /* Shift PT offset to bytes */
    orr x10, x10, x11               /* Compute descriptor address */
    sub x13, x13, #1                /* Decrease level */
    cbz x13, map_done               /* If we reached level 0 then finalize */
    ldr x12, [x10]                  /* Otherwise, fetch the descriptor */
    and x11, x12, #0x1              /* Filter valid bit */
    cbz x11, map_alloc_page         /* If the descriptor is valid then next */
    b map_loop                      /* Next level */
map_alloc_page:
    mov x11, x0                     /* Save VA across call */
    bl allocate_pa                  /* Allocate a PT phys page */
    mov x12, x0                     /* Got a PA */
    mov x0, x11                     /* Restore VA */
    orr x12, x12, #PTE_TABLE        /* This is a table entry */
    str x12, [x10]                  /* Fill in PT entry */
    b map_loop                      /* Next level */
map_done:
    orr x12, x1, x2                 /* Create PTE: PA + pgprop */
    str x12, [x10]                  /* Fill in PT entry */
    ldp x13, x14, [sp], #16
    ldp x11, x12, [sp], #16
    ldp x30, x10, [sp], #16
    ret

.globl unmap_va
/* unmap_va(VA) */
unmap_va:
    stp x30, x10, [sp, #-16]!
    stp x11, x12, [sp, #-16]!
    stp x13, x14, [sp, #-16]!
    ldr x12, =PT_BASE
    mov x13, #0x4
    mov x14, #39
unmap_loop:
    and x11, x12, #~0xFFF           /* Strip off descriptor non-address bits */
    lsr x10, x0, x14                /* Shift out VA bits for the level */
    sub x14, x14, #9                /* Update shift amount for next level */
    and x10, x10, #0x1FF            /* Filter top VA bits for PT offset */
    lsl x10, x10, #3                /* Shift PT offset to bytes */
    orr x10, x10, x11               /* Compute descriptor address */
    sub x13, x13, #1                /* Decrease level */
    cbz x13, unmap_page             /* If we reached level 0 then finalize */
    ldr x12, [x10]                  /* Otherwise, fetch the descriptor */
    and x11, x12, #0x1              /* Filter valid bit */
    cbz x11, unmap_done             /* Assume an invalid PT page means done */
    b unmap_loop                    /* Next level */
unmap_page:
    mov x12, #0                     /* Clear the page PTE */
    str x12, [x10]                  /* Fill in PT entry */
unmap_done:
    ldp x13, x14, [sp], #16
    ldp x11, x12, [sp], #16
    ldp x30, x10, [sp], #16
    ret

.globl map_va
/* map_va(VA, pgprop) */
map_va:
    str x30, [sp, #-8]!
    stp x2, x10, [sp, #-16]!
    mov x10, x0
    bl allocate_pa
    mov x2, x0
    mov x0, x10
    bl map_va_to_pa
    ldp x2, x10, [sp], #16
    ldr x30, [sp], #8
    ret

.globl map_pa
/* map_pa(PA, pgprop) */
map_pa:
    stp x30, x2, [sp, #-16]!
    mov x2, x0
    bl map_va_to_pa
    ldp x30, x2, [sp], #16
    ret

.globl map_va_to_pa_range
/* map_va_to_pa_range(VA, pgprop, PA, len) */
map_va_to_pa_range:
    stp x30, x3, [sp, #-16]!
    stp x0, x2, [sp, #-16]!
    add x3, x3, #0xFFF
    and x3, x3, #~0xFFF
map_va_to_pa_loop:
    cbz x3, map_va_to_pa_done
    bl map_va_to_pa
    add x0, x0, #0x1000
    add x2, x2, #0x1000
    sub x3, x3, #0x1000
    b map_va_to_pa_loop
map_va_to_pa_done:
    ldp x0, x2, [sp], #16
    ldp x30, x3, [sp], #16
    ret

/* map_pa_range(PA, pgprop, len) */
map_pa_range:
    str x30, [sp, #-8]!
    stp x0, x2, [sp, #-16]!
    add x2, x2, #0xFFF
    and x2, x2, #~0xFFF
map_pa_loop:
    cbz x2, map_pa_done
    bl map_pa
    add x0, x0, #0x1000
    sub x2, x2, #0x1000
    b map_pa_loop
map_pa_done:
    ldp x0, x2, [sp], #16
    ldr x30, [sp], #8
    ret

/* map_va_range(VA, pgprop, len) */
map_va_range:
    str x30, [sp, #-8]!
    stp x0, x2, [sp, #-16]!
    add x2, x2, #0xFFF
    and x2, x2, #~0xFFF
map_va_loop:
    cbz x2, map_va_done
    bl map_va
    add x0, x0, #0x1000
    sub x2, x2, #0x1000
    b map_va_loop
map_va_done:
    ldp x0, x2, [sp], #16
    ldr x30, [sp], #8
    ret

/* unmap_va_range(VA, len) */
unmap_va_range:
    str x30, [sp, #-8]!
    stp x0, x1, [sp, #-16]!
    add x1, x1, #0xFFF
    and x1, x1, #~0xFFF
unmap_va_loop:
    cbz x1, unmap_va_done
    bl unmap_va
    add x0, x0, #0x1000
    sub x1, x1, #0x1000
    b unmap_va_loop
unmap_va_done:
    ldp x0, x1, [sp], #16
    ldr x30, [sp], #8
    ret

/* memcpy(dest, src) */
memcpy:
    cmp x2, #0
    ble memcpy_done
    ldr x10, [x1], #8
    str x10, [x0], #8
    subs x2, x2, #8
    b memcpy
memcpy_done:
    ret
