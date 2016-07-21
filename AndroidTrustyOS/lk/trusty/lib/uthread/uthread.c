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

#include <uthread.h>
#include <stdlib.h>
#include <string.h>
#include <compiler.h>
#include <assert.h>
#include <lk/init.h>
#include <trace.h>

#include <kernel/mutex.h>

#include <arch/uthread_mmu.h>  // for MAX_USR_VA

/* Global list of all userspace threads */
static struct list_node uthread_list;

/* Monotonically increasing thread id for now */
static uint32_t next_utid;
static spin_lock_t uthread_lock;

static inline void mmap_lock(uthread_t *ut)
{
	DEBUG_ASSERT(ut);
	mutex_acquire(&ut->mmap_lock);
}

static inline void mmap_unlock(uthread_t *ut)
{
	DEBUG_ASSERT(ut);
	mutex_release(&ut->mmap_lock);
}


/* TODO: implement a utid hashmap */
static uint32_t uthread_alloc_utid(void)
{
	spin_lock_saved_state_t state;

	spin_lock_save(&uthread_lock, &state, SPIN_LOCK_FLAG_INTERRUPTS);
	next_utid++;
	spin_unlock_restore(&uthread_lock, state, SPIN_LOCK_FLAG_INTERRUPTS);

	return next_utid;
}

/* TODO: */
static void uthread_free_utid(uint32_t utid)
{
}


static vaddr_t uthread_find_va_space_ns(uthread_t *ut, size_t size, u_int align)
{
	vaddr_t start, end;
	uthread_map_t *mp;

	/* get first suitable address */
	start = ROUNDDOWN(MAX_USR_VA - size, align);
	end = start + size;

	mp = list_peek_tail_type (&ut->map_list, uthread_map_t, node);
	while (mp) {
		if (start >= mp->vaddr + mp->size) { /* found gap */

			if (start >= ut->ns_va_bottom)
				break; /* the whole map fits above ns_va_bottom */

			/* check if we can move ns_va_bottom */
			DEBUG_ASSERT((mp->flags & UTM_NS_MEM) == 0);
			if (ROUNDDOWN(start, UT_MAP_ALIGN_1MB) < mp->vaddr + mp->size)
				return (vaddr_t) NULL; /*  nope, we can't */

			break;
		}

		if (!(mp->flags & UTM_NS_MEM))
			return (vaddr_t) NULL;  /* next mp is non-NS */

		start = MIN(start, ROUNDDOWN((mp->vaddr - size), align));
		end = start + size;

		/* get prev list item */
		mp = list_prev_type(&ut->map_list, &mp->node, uthread_map_t, node);
	}

	if (start < ut->start_stack || start > end)
		return (vaddr_t) NULL;

	return start;
}

static vaddr_t uthread_find_va_space_sec(uthread_t *ut, size_t size, u_int align)
{
	vaddr_t start, end;
	uthread_map_t *mp;

	start = ROUNDUP(ut->start_stack, align);
	end = start + size;

	/* find first fit */
	list_for_every_entry(&ut->map_list, mp, uthread_map_t, node) {
		if (end <= mp->vaddr)
			break;

		start = MAX(start, ROUNDUP((mp->vaddr + mp->size), align));
		end = start + size;
	}

	if (end > ut->ns_va_bottom || start > end)
		return (vaddr_t) NULL;

	return start;
}

static vaddr_t uthread_find_va_space(uthread_t *ut, size_t size,
		u_int flags, u_int align)
{
	if (flags & UTM_NS_MEM)
		return uthread_find_va_space_ns(ut, size, align);
	else
		return uthread_find_va_space_sec(ut, size, align);
}

static status_t uthread_map_alloc(uthread_t *ut, uthread_map_t **mpp,
		vaddr_t vaddr, paddr_t *pfn_list, size_t size, u_int flags,
		u_int align)
{
	uthread_map_t *mp, *mp_lst;
	status_t err = NO_ERROR;
	uint32_t npages;

	ASSERT(!(size & (PAGE_SIZE - 1)));

	if (vaddr + size <= vaddr)
		return ERR_INVALID_ARGS;

	if (flags & UTM_PHYS_CONTIG)
		npages = 1;
	else
		npages = (size / PAGE_SIZE);

	mp = malloc(sizeof(uthread_map_t) + (npages * sizeof(mp->pfn_list[0])));
	if (!mp) {
		err = ERR_NO_MEMORY;
		goto err_out;
	}

	mp->vaddr = vaddr;
	mp->size = size;
	mp->flags = flags;
	mp->align = align;
	memcpy(mp->pfn_list, pfn_list, npages*sizeof(paddr_t));


	vaddr_t new_ns = ut->ns_va_bottom;
	if ((mp->flags & UTM_NS_MEM) && (mp->vaddr < ut->ns_va_bottom)) {
		/* Move ns_va_bottom down.
		   It is verified during NS VA allocation */
		new_ns = ROUNDDOWN(mp->vaddr, UT_MAP_ALIGN_1MB);
	}

	list_for_every_entry(&ut->map_list, mp_lst, uthread_map_t, node) {
		if (mp_lst->vaddr > mp->vaddr) {
			if((mp->vaddr + mp->size) > mp_lst->vaddr) {
				err = ERR_INVALID_ARGS;
				goto err_free_mp;
			}
			ut->ns_va_bottom = new_ns;
			list_add_before(&mp_lst->node, &mp->node);
			goto out;
		} else {
			if (mp->vaddr < (mp_lst->vaddr + mp_lst->size)) {
				err = ERR_INVALID_ARGS;
				goto err_free_mp;
			}
		}
	}

	ut->ns_va_bottom = new_ns;
	list_add_tail(&ut->map_list, &mp->node);
out:
	if (mpp)
		*mpp = mp;
	return NO_ERROR;

err_free_mp:
	free(mp);
err_out:
	if (mpp)
		*mpp = NULL;
	return err;
}

static uthread_map_t *uthread_map_find(uthread_t *ut, vaddr_t vaddr, size_t size)
{
	uthread_map_t *mp = NULL;

	if (vaddr + size < vaddr)
		return NULL;

	/* TODO: Fuzzy comparisions for now */
	list_for_every_entry(&ut->map_list, mp, uthread_map_t, node) {
		if ((mp->vaddr <= vaddr) &&
		    (vaddr < mp->vaddr + mp->size) &&
		    ((mp->vaddr + mp->size) >= (vaddr + size))) {
			return mp;
		}
	}

	return NULL;
}

/* caller ensures mp is in the mapping list */
static void uthread_map_remove(uthread_t *ut, uthread_map_t *mp)
{
	if (mp->flags & UTM_NS_MEM) {
		uthread_map_t *item;

		/* check if we need to move ns_va_bottom up */
		item = list_prev_type(&ut->map_list, &mp->node,
		                       uthread_map_t, node);

		if (!item || (item->flags & UTM_NS_MEM) == 0) {
			/* removing bottom NS mapping, so next ns_va_bottom is
			   rounded down to 1M start of next in list ns segment
			   or MAX_USR_VA if none exists */
			item = list_next_type(&ut->map_list, &mp->node,
		                       uthread_map_t, node);
			if (item) {
				DEBUG_ASSERT(item->flags & UTM_NS_MEM);
				ut->ns_va_bottom = ROUNDDOWN(item->vaddr,
				                          UT_MAP_ALIGN_1MB);
			} else {
				ut->ns_va_bottom = MAX_USR_VA;
			}
		}
	}

	list_delete(&mp->node);
	free(mp);
}

static void uthread_free_maps(uthread_t *ut)
{
	uthread_map_t *mp, *tmp;
	list_for_every_entry_safe(&ut->map_list, mp, tmp,
			uthread_map_t, node) {
		list_delete(&mp->node);
		free(mp);
	}
}

uthread_t *uthread_create(const char *name, vaddr_t entry, int priority,
		vaddr_t start_stack, size_t stack_size, void *private_data)
{
	uthread_t *ut = NULL;
	status_t err;
	vaddr_t stack_bot;
	spin_lock_saved_state_t state;

	ut = (uthread_t *)calloc(1, sizeof(uthread_t));
	if (!ut)
		goto err_done;

	list_initialize(&ut->map_list);
	mutex_init(&ut->mmap_lock);

	ut->id = uthread_alloc_utid();
	ut->private_data = private_data;
	ut->entry = entry;
	ut->ns_va_bottom = MAX_USR_VA;

	/* Allocate and map in a stack region */
	ut->stack = memalign(PAGE_SIZE, stack_size);
	if(!ut->stack)
		goto err_free_ut;

	stack_bot = start_stack - stack_size;
	err = uthread_map_contig(ut, &stack_bot, vaddr_to_paddr(ut->stack),
				stack_size,
				UTM_W | UTM_R | UTM_STACK | UTM_FIXED,
				UT_MAP_ALIGN_4KB);
	if (err)
		goto err_free_ut_stack;

	ut->start_stack = start_stack;

	ut->thread = thread_create(name,
			(thread_start_routine)arch_uthread_startup,
			NULL,
			priority,
			DEFAULT_STACK_SIZE);
	if (!ut->thread)
		goto err_free_ut_maps;

	err = arch_uthread_create(ut);
	if (err)
		goto err_free_ut_maps;

	/* store user thread struct into TLS slot 0 */
	ut->thread->tls[TLS_ENTRY_UTHREAD] = (uintptr_t) ut;

	/* Put it in global uthread list */
	spin_lock_save(&uthread_lock, &state, SPIN_LOCK_FLAG_INTERRUPTS);
	list_add_head(&uthread_list, &ut->uthread_list_node);
	spin_unlock_restore(&uthread_lock, state, SPIN_LOCK_FLAG_INTERRUPTS);

	return ut;

err_free_ut_maps:
	uthread_free_maps(ut);

err_free_ut_stack:
	free(ut->stack);

err_free_ut:
	uthread_free_utid(ut->id);
	free(ut);

err_done:
	return NULL;
}

status_t uthread_start(uthread_t *ut)
{
	if (!ut || !ut->thread)
		return ERR_INVALID_ARGS;

	return thread_resume(ut->thread);
}

void __NO_RETURN uthread_exit(int retcode)
{
	uthread_t *ut;

	ut = uthread_get_current();
	if (ut) {
		uthread_free_maps(ut);
		free(ut->stack);
		arch_uthread_free(ut);
		uthread_free_utid(ut->id);
		free(ut);
	} else {
		TRACEF("WARNING: unexpected call on kernel thread %s!",
				get_current_thread()->name);
	}

	thread_exit(retcode);
}

void uthread_context_switch(thread_t *oldthread, thread_t *newthread)
{
	uthread_t *old_ut = (uthread_t *)oldthread->tls[TLS_ENTRY_UTHREAD];
	uthread_t *new_ut = (uthread_t *)newthread->tls[TLS_ENTRY_UTHREAD];

	arch_uthread_context_switch(old_ut, new_ut);
}

static status_t uthread_map_locked(uthread_t *ut, vaddr_t *vaddrp,
		paddr_t *pfn_list, size_t size, u_int flags, u_int align)
{
	uthread_map_t *mp = NULL;
	status_t err = NO_ERROR;

	if (!ut || !pfn_list || !vaddrp) {
		err = ERR_INVALID_ARGS;
		goto done;
	}

	if((size & (PAGE_SIZE - 1))) {
		err = ERR_NOT_VALID;
		goto done;
	}

	if(!(flags & UTM_FIXED)) {
		*vaddrp = uthread_find_va_space(ut, size, flags, align);

		if (!(*vaddrp)) {
			err = ERR_NO_MEMORY;
			goto done;
		}
	}

	err = uthread_map_alloc(ut, &mp, *vaddrp, pfn_list, size, flags, align);
	if(err)
		goto done;

	err = arch_uthread_map(ut, mp);
done:
	return err;
}

status_t uthread_map(uthread_t *ut, vaddr_t *vaddrp, paddr_t *pfn_list,
		size_t size, u_int flags, u_int align)
{
	status_t err;

	mmap_lock(ut);
	err = uthread_map_locked(ut, vaddrp, pfn_list, size, flags, align);
	mmap_unlock(ut);
	return err;
}

static status_t uthread_unmap_locked(uthread_t *ut, vaddr_t vaddr, size_t size)
{
	uthread_map_t *mp;
	status_t err = NO_ERROR;

	if (!ut || !vaddr) {
		err = ERR_INVALID_ARGS;
		goto done;
	}

	mp = uthread_map_find(ut, vaddr, size);
	if(!mp) {
		err = ERR_NOT_FOUND;
		goto done;
	}

	err = arch_uthread_unmap(ut, mp);
	if (err)
		goto done;

	uthread_map_remove(ut, mp);
done:
	return err;
}

status_t uthread_unmap(uthread_t *ut, vaddr_t vaddr, size_t size)
{
	status_t err;

	mmap_lock(ut);
	err = uthread_unmap_locked(ut, vaddr, size);
	mmap_unlock(ut);
	return err;
}

bool uthread_is_valid_range(uthread_t *ut, vaddr_t vaddr, size_t size)
{
	bool ret;

	mmap_lock(ut);
	ret = uthread_map_find(ut, vaddr, size) != NULL ? true : false;
	mmap_unlock(ut);
	return ret;
}

#if UTHREAD_WITH_MEMORY_MAPPING_SUPPORT

static inline void mmap_lock_pair (uthread_t *source, uthread_t *target)
{
	DEBUG_ASSERT(source != target);

	/* source can be NULL */
	/* target cannot be NULL */

	if (target < source)
		mmap_lock(target);

	if (source)
		mmap_lock(source);

	if (target > source)
		mmap_lock(target);
}

static inline void mmap_unlock_pair (uthread_t *source, uthread_t *target)
{
	DEBUG_ASSERT(source != target);

	/* source can be NULL */
	/* target cannot be NULL */

	if (target > source)
		mmap_unlock(target);

	if (source)
		mmap_unlock(source);

	if (target < source)
		mmap_unlock(target);
}

status_t uthread_virt_to_phys(uthread_t *ut, vaddr_t vaddr, paddr_t *paddr)
{
	uthread_map_t *mp;
	u_int offset = vaddr & (PAGE_SIZE -1);
	status_t err;

	mmap_lock(ut);
	mp = uthread_map_find(ut, vaddr, 0);
	if (!mp) {
		err = ERR_INVALID_ARGS;
		goto err_out;
	}

	if (mp->flags & UTM_PHYS_CONTIG) {
		*paddr = mp->pfn_list[0] + (vaddr - mp->vaddr);
	} else {
		uint32_t pg = (vaddr - mp->vaddr) / PAGE_SIZE;
		*paddr = mp->pfn_list[pg] + offset;
	}
	err = NO_ERROR;
err_out:
	mmap_unlock(ut);
	return err;
}

status_t uthread_grant_pages(uthread_t *ut_target, ext_vaddr_t vaddr_src,
		size_t size, u_int flags, vaddr_t *vaddr_target, bool ns_src,
		uint64_t *ns_page_list)
{
	u_int align, npages;
	paddr_t *pfn_list;
	status_t err;
	u_int offset;

	if (size == 0) {
		*vaddr_target = 0;
		return 0;
	}

	uthread_t *ut_src = ns_src ? NULL : uthread_get_current();

	mmap_lock_pair(ut_src, ut_target);

	offset = vaddr_src & (PAGE_SIZE -1);
	vaddr_src = ROUNDDOWN(vaddr_src, PAGE_SIZE);

	if (ns_src) {
		if (!ns_page_list) {
			err = ERR_INVALID_ARGS;
			goto err_out;
		}

		align = UT_MAP_ALIGN_4KB;
		flags |= UTM_NS_MEM;
	} else {
		uthread_map_t *mp_src;

		/* ns_page_list should only be passes for NS src -> secure target
		 * mappings
		 */
		ASSERT(!ns_page_list);

		/* Only a vaddr_t sized vaddr_src is supported
		 * for secure src -> secure target mappings.
		 */
		ASSERT(vaddr_src == (vaddr_t)vaddr_src);
		mp_src = uthread_map_find(ut_src, (vaddr_t)vaddr_src, size);
		if (!mp_src) {
			err = ERR_INVALID_ARGS;
			goto err_out;
		}

		if (mp_src->flags & UTM_NS_MEM)
			flags = flags | UTM_NS_MEM;

		align = mp_src->align;
	}

	size = ROUNDUP((size + offset), PAGE_SIZE);

	npages = size / PAGE_SIZE;
	if (npages == 1)
		flags |= UTM_PHYS_CONTIG;

	*vaddr_target = uthread_find_va_space(ut_target, size, flags, align);
	if (!(*vaddr_target)) {
		err = ERR_NO_MEMORY;
		goto err_out;
	}

	pfn_list = malloc(npages * sizeof(paddr_t));
	if (!pfn_list) {
		err = ERR_NO_MEMORY;
		goto err_out;
	}

	/* translate and map */
	err = arch_uthread_translate_map(ut_target, vaddr_src, *vaddr_target,
			pfn_list, npages, flags, ns_src, ns_page_list);

	if (err != NO_ERROR)
		goto err_free_pfn_list;

	err = uthread_map_alloc(ut_target, NULL, *vaddr_target, pfn_list,
			size, flags, align);

	/* Add back the offset after translation and mapping */
	*vaddr_target += offset;

err_free_pfn_list:
	if (pfn_list)
		free(pfn_list);
err_out:
	mmap_unlock_pair(ut_src, ut_target);
	return err;
}

status_t uthread_revoke_pages(uthread_t *ut, vaddr_t vaddr, size_t size)
{
	u_int offset = vaddr & (PAGE_SIZE - 1);

	if (size == 0)
		return 0;

	vaddr = ROUNDDOWN(vaddr, PAGE_SIZE);
	size  = ROUNDUP(size + offset, PAGE_SIZE);

	return uthread_unmap(ut, vaddr, size);
}
#endif

static void uthread_init(uint level)
{
	list_initialize(&uthread_list);
	arch_uthread_init();
}

/* this has to come up early because we have to reinitialize the MMU on
 * some arch's
 */
LK_INIT_HOOK(libuthread, uthread_init, LK_INIT_LEVEL_ARCH_EARLY);
