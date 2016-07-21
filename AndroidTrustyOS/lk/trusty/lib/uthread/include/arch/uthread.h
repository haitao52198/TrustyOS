/*
 * Copyright (c) 2013, Google Inc. All rights reserved
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

#ifndef __ARCH_UTHREAD_H
#define __ARCH_UTHREAD_H

#include <arch/arch_uthread.h>
#include <err.h>
#include <stdbool.h>
#include <sys/types.h>

struct uthread;
struct uthread_map;

/* Holds a vaddr large enough for 32 or 64 bit clients */
typedef uint64_t ext_vaddr_t;
typedef uint32_t user_addr_t;

void arch_uthread_init(void);

status_t arch_uthread_create(struct uthread *ut);
void arch_uthread_free(struct uthread *ut);

void arch_uthread_startup(void);
void arch_uthread_context_switch(struct uthread *old_ut, struct uthread *new_ut);

status_t arch_uthread_map(struct uthread *ut, struct uthread_map *mp);
status_t arch_uthread_unmap(struct uthread *ut, struct uthread_map *mp);

status_t arch_copy_from_user(void *kdest, user_addr_t usrc, size_t len);
status_t arch_copy_to_user(user_addr_t udest, const void *ksrc, size_t len);
ssize_t arch_strlcpy_from_user(char *kdst, user_addr_t usrc, size_t len);

#if UTHREAD_WITH_MEMORY_MAPPING_SUPPORT
status_t arch_uthread_translate_map(struct uthread *ut_target, ext_vaddr_t vaddr_src,
		vaddr_t vaddr_target, paddr_t *pfn_list,
		uint32_t npages, u_int flags, bool ns_src,
		uint64_t *ns_pte_list);
#endif

#endif /* __ARCH_UTHREAD_H */
