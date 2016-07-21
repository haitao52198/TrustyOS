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

#ifndef __UTHREAD_H
#define __UTHREAD_H

#include <sys/types.h>
#include <list.h>
#include <compiler.h>
#include <err.h>
#include <kernel/thread.h>
#include <kernel/mutex.h>
#include <arch/uthread.h>

#if WITH_KERNEL_VM
#include <kernel/vm.h>
#endif

typedef struct uthread_map
{
	vaddr_t vaddr;
	size_t  size;
	u_int  flags;
	u_int align;
	struct list_node node;
	paddr_t pfn_list[];
} uthread_map_t;

/*
 * user thread virtual memory layout:
 *
 * +-----------------------+ 0xFFFFFFFF
 * |                       |
 * |        kernel         |
 * |                       |
 * +-----------------------+ MEMBASE
 * |                       |
 * |                       |
 * |    other mappings     |
 * |                       |
 * |                       |
 * +-----------------------+ start_stack
 * |        stack          |
 * |     (grows down)      |
 * +-----------------------+
 * |        heap           |
 * +-----------------------+ start_brk
 * |        .bss           |
 * +-----------------------+
 * |        .data          |
 * +-----------------------+ start_data
 * |                       |
 * |     .text, .rodata    |
 * |                       |
 * +-----------------------+ start_code
 * |                       |
 * |    unmapped region    |
 * +-----------------------+ 0x00000000
 *
 * The actual boundaries between various segments are
 * determined by the binary image loader.
 *
 */
typedef struct uthread
{
	vaddr_t start_stack;

	vaddr_t entry;
	void *stack;

	struct list_node map_list;
	mutex_t mmap_lock;
	vaddr_t ns_va_bottom;

	/* uthread ID, unique per thread */
	uint32_t id;

	void *page_table;

	thread_t *thread;
	struct list_node uthread_list_node;
	void *private_data;

	struct arch_uthread arch;
} uthread_t;

/* uthread generic mapping flags */
enum
{
	UTM_R		= 1 << 0,
	UTM_W		= 1 << 1,
	UTM_X		= 1 << 2,
	UTM_NONE	= 1 << 3,
	UTM_STACK	= 1 << 4,
	UTM_PHYS_CONTIG	= 1 << 5,
	UTM_NS_MEM	= 1 << 6,
	UTM_IO		= 1 << 7,
	UTM_FIXED	= 1 << 8,
};

/* uthread mapping alignments */
#define UT_MAP_ALIGN_4KB	(4UL * 1024)
#define UT_MAP_ALIGN_1MB	(1UL * 1024 * 1024)
#define UT_MAP_ALIGN_DEFAULT	UT_MAP_ALIGN_4KB

/* Create a new user thread */
uthread_t *uthread_create(const char *name, vaddr_t entry, int priority,
		vaddr_t stack_top, size_t stack_size, void *private_data);

/* Start the user thread */
status_t uthread_start(uthread_t *ut);

/* Exit current uthread */
void uthread_exit(int retcode) __NO_RETURN;

/* Map a region of memory in the uthread */
status_t uthread_map(uthread_t *ut, vaddr_t *vaddrp, paddr_t *pfn_list,
		size_t size, u_int flags, u_int align);

/* Unmap a region of memory */
status_t uthread_unmap(uthread_t *ut, vaddr_t vaddr, size_t size);

/* Check if the given user address range has a valid mapping */
bool uthread_is_valid_range(uthread_t *ut, vaddr_t vaddr, size_t size);

static inline status_t copy_from_user(void *kdest, user_addr_t usrc, size_t len)
{
	return arch_copy_from_user(kdest, usrc, len);
}

static inline status_t copy_to_user(user_addr_t udest, const void *ksrc, size_t len)
{
	return arch_copy_to_user(udest, ksrc, len);
}

static inline ssize_t  strncpy_from_user(char *kdest, user_addr_t usrc, size_t len)
{
	/* wrapper for now, the old strncpy_from_user was closer to strlcpy than
	 * strncpy behaviour, but could return an unterminated string */
	return arch_strlcpy_from_user(kdest, usrc, len);
}

static inline ssize_t  strlcpy_from_user(char *kdest, user_addr_t usrc, size_t len)
{
	return arch_strlcpy_from_user(kdest, usrc, len);
}

static inline __ALWAYS_INLINE
status_t uthread_map_contig(uthread_t *ut, vaddr_t *vaddrp, paddr_t paddr,
		size_t size, u_int flags, u_int align)
{
	flags = flags | UTM_PHYS_CONTIG;
	return uthread_map(ut, vaddrp, &paddr, size, flags, align);
}

static inline uthread_t *uthread_get_current(void)
{
	return (uthread_t *)tls_get(TLS_ENTRY_UTHREAD);
}

#if UTHREAD_WITH_MEMORY_MAPPING_SUPPORT
/* Translate virtual address to physical address */
status_t uthread_virt_to_phys(uthread_t *ut, vaddr_t vaddr, paddr_t *paddr);

/* Grant pages from current context into target uthread */
status_t uthread_grant_pages(uthread_t *ut_target, ext_vaddr_t vaddr_src,
		size_t size, u_int flags, vaddr_t *vaddr_target, bool ns_src,
		uint64_t *ns_page_list);

/* Revoke mappings from a previous grant */
status_t uthread_revoke_pages(uthread_t *ut, vaddr_t vaddr, size_t size);
#endif

#endif /* __UTHREAD_H */
