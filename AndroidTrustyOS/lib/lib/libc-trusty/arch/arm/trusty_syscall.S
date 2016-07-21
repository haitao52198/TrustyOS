/*
 * Copyright (c) 2013 Google Inc. All rights reserved
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files
 * (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* This file is auto-generated. !!! DO NOT EDIT !!! */

#include <asm.h>
#include <trusty_syscalls.h>

.section .text.write
FUNCTION(write)
    ldr     r12, =__NR_write
    swi     #0
    bx      lr

.section .text.brk
FUNCTION(brk)
    ldr     r12, =__NR_brk
    swi     #0
    bx      lr

.section .text.exit_group
FUNCTION(exit_group)
    ldr     r12, =__NR_exit_group
    swi     #0
    bx      lr

.section .text.read
FUNCTION(read)
    ldr     r12, =__NR_read
    swi     #0
    bx      lr

.section .text.ioctl
FUNCTION(ioctl)
    ldr     r12, =__NR_ioctl
    swi     #0
    bx      lr

.section .text.nanosleep
FUNCTION(nanosleep)
    ldr     r12, =__NR_nanosleep
    swi     #0
    bx      lr

.section .text.gettime
FUNCTION(gettime)
    ldr     r12, =__NR_gettime
    swi     #0
    bx      lr

.section .text.mmap
FUNCTION(mmap)
    ldr     r12, =__NR_mmap
    swi     #0
    bx      lr

.section .text.munmap
FUNCTION(munmap)
    ldr     r12, =__NR_munmap
    swi     #0
    bx      lr

.section .text.prepare_dma
FUNCTION(prepare_dma)
    ldr     r12, =__NR_prepare_dma
    swi     #0
    bx      lr

.section .text.finish_dma
FUNCTION(finish_dma)
    ldr     r12, =__NR_finish_dma
    swi     #0
    bx      lr

.section .text.port_create
FUNCTION(port_create)
    ldr     r12, =__NR_port_create
    swi     #0
    bx      lr

.section .text.connect
FUNCTION(connect)
    ldr     r12, =__NR_connect
    swi     #0
    bx      lr

.section .text.accept
FUNCTION(accept)
    ldr     r12, =__NR_accept
    swi     #0
    bx      lr

.section .text.close
FUNCTION(close)
    ldr     r12, =__NR_close
    swi     #0
    bx      lr

.section .text.set_cookie
FUNCTION(set_cookie)
    ldr     r12, =__NR_set_cookie
    swi     #0
    bx      lr

.section .text.wait
FUNCTION(wait)
    ldr     r12, =__NR_wait
    swi     #0
    bx      lr

.section .text.wait_any
FUNCTION(wait_any)
    ldr     r12, =__NR_wait_any
    swi     #0
    bx      lr

.section .text.get_msg
FUNCTION(get_msg)
    ldr     r12, =__NR_get_msg
    swi     #0
    bx      lr

.section .text.read_msg
FUNCTION(read_msg)
    ldr     r12, =__NR_read_msg
    swi     #0
    bx      lr

.section .text.put_msg
FUNCTION(put_msg)
    ldr     r12, =__NR_put_msg
    swi     #0
    bx      lr

.section .text.send_msg
FUNCTION(send_msg)
    ldr     r12, =__NR_send_msg
    swi     #0
    bx      lr
