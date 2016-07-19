#define __ASSEMBLY__

.section .text
.globl __smc
__smc:
    smc #0
    ret

.globl __svc
__svc:
    svc #0
    ret

.macro READ_REG reg
.globl read_\reg
read_\reg:
    mrs x0, \reg
    ret
.endm

.macro WRITE_REG reg
.globl write_\reg
write_\reg:
    msr \reg, x0
    ret
.endm

READ_REG currentel
WRITE_REG currentel

READ_REG scr_el3
WRITE_REG scr_el3

READ_REG sder32_el3
WRITE_REG sder32_el3

READ_REG cptr_el3
WRITE_REG cptr_el3

READ_REG cpacr_el1
WRITE_REG cpacr_el1

READ_REG sctlr_el1
WRITE_REG sctlr_el1

READ_REG sctlr_el3
WRITE_REG sctlr_el3

.globl __set_exception_return
__set_exception_return:
    str x30, [sp, #-8]!
    mrs x7, currentel
    cmp x7, #0x4
    b.eq __set_exception_return_el1
    cmp x7, #0x8
    b.eq __set_exception_return_el2
__set_exception_return_el3:
    msr elr_el3, x0
    b __set_exception_return_done
__set_exception_return_el2:
    msr elr_el2, x0
    b __set_exception_return_done
__set_exception_return_el1:
    msr elr_el1, x0
__set_exception_return_done:
    ldr x30, [sp], #8
    ret

.globl __exception_return
__exception_return:
    mrs x7, currentel
    cmp x7, #0x4
    b.eq __exception_return_el1
    cmp x7, #0x8
    b.eq __exception_return_el2
__exception_return_el3:
    msr elr_el3, x0
    msr spsr_el3, x1
    b __exception_return_done
__exception_return_el2:
    msr elr_el2, x0
    msr spsr_el2, x1
    b __exception_return_done
__exception_return_el1:
    msr elr_el1, x0
    msr spsr_el1, x1
__exception_return_done:
    eret

#undef __ASSEMBLY__
