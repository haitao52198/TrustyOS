/*
 * Copyright (c) 2012-2013, NVIDIA CORPORATION. All rights reserved
 * Copyright (c) 2013, Google, Inc. All rights reserved
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

#define DEBUG_LOAD_TRUSTY_APP	1

#include <arch.h>
#include <assert.h>
#include <compiler.h>
#include <debug.h>
#include "elf.h"
#include <err.h>
#include <kernel/mutex.h>
#include <kernel/thread.h>
#include <malloc.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <uthread.h>
#include <lk/init.h>

#include <lib/trusty/trusty_app.h>

/*
 * Layout of .trusty_app.manifest section in the trusted application is the
 * required UUID followed by an abitrary number of configuration options.
 *
 * Note: Ensure that the manifest definition is kept in sync with the
 * one userspace uses to build the trusty apps.
 */

enum {
	TRUSTY_APP_CONFIG_KEY_MIN_STACK_SIZE	= 1,
	TRUSTY_APP_CONFIG_KEY_MIN_HEAP_SIZE	= 2,
	TRUSTY_APP_CONFIG_KEY_MAP_MEM		= 3,
};

typedef struct trusty_app_manifest {
	uuid_t		uuid;
	char   		app_name[256];
	uint32_t	config_options[];
} trusty_app_manifest_t;

#define MAX_TRUSTY_APP_COUNT	(PAGE_SIZE / sizeof(trusty_app_t))

#define TRUSTY_APP_START_ADDR	0x8000
#define TRUSTY_APP_STACK_TOP	0x1000000 /* 16MB */

#define PAGE_MASK		(PAGE_SIZE - 1)

static u_int trusty_app_count;
static trusty_app_t *trusty_app_list;

static char *trusty_app_image_start;
static char *trusty_app_image_end;
static u_int trusty_app_image_size;

extern intptr_t __trusty_app_start;
extern intptr_t __trusty_app_end;

static bool apps_started;
static mutex_t apps_lock = MUTEX_INITIAL_VALUE(apps_lock);
static struct list_node app_notifier_list = LIST_INITIAL_VALUE(app_notifier_list);
uint als_slot_cnt;

#define PRINT_TRUSTY_APP_UUID(tid,u)					\
	dprintf(SPEW,							\
		"trusty_app %d uuid: 0x%x 0x%x 0x%x 0x%x%x 0x%x%x%x%x%x%x\n",\
		tid,							\
		(u)->time_low, (u)->time_mid,				\
		(u)->time_hi_and_version,				\
		(u)->clock_seq_and_node[0],				\
		(u)->clock_seq_and_node[1],				\
		(u)->clock_seq_and_node[2],				\
		(u)->clock_seq_and_node[3],				\
		(u)->clock_seq_and_node[4],				\
		(u)->clock_seq_and_node[5],				\
		(u)->clock_seq_and_node[6],				\
		(u)->clock_seq_and_node[7]);

static void finalize_registration(void)
{
	mutex_acquire(&apps_lock);
	apps_started = true;
	mutex_release(&apps_lock);
}

status_t trusty_register_app_notifier(trusty_app_notifier_t *n)
{
	status_t ret = NO_ERROR;

	mutex_acquire(&apps_lock);
	if (!apps_started)
		list_add_tail(&app_notifier_list, &n->node);
	else
		ret = ERR_ALREADY_STARTED;
	mutex_release(&apps_lock);
	return ret;
}

int trusty_als_alloc_slot(void)
{
	int ret;

	mutex_acquire(&apps_lock);
	if (!apps_started)
		ret = ++als_slot_cnt;
	else
		ret = ERR_ALREADY_STARTED;
	mutex_release(&apps_lock);
	return ret;
}


static void load_app_config_options(intptr_t trusty_app_image_addr,
		trusty_app_t *trusty_app, Elf32_Shdr *shdr)
{
	char  *manifest_data;
	u_int *config_blob, config_blob_size;
	u_int i;
	u_int trusty_app_idx;

	/* have to at least have a valid UUID */
	ASSERT(shdr->sh_size >= sizeof(uuid_t));

	/* init default config options before parsing manifest */
	trusty_app->props.min_heap_size = 4 * PAGE_SIZE;
	trusty_app->props.min_stack_size = DEFAULT_STACK_SIZE;

	trusty_app_idx = trusty_app - trusty_app_list;

	manifest_data = (char *)(trusty_app_image_addr + shdr->sh_offset);

	memcpy(&trusty_app->props.uuid,
	       (uuid_t *)manifest_data,
	       sizeof(uuid_t));

	PRINT_TRUSTY_APP_UUID(trusty_app_idx, &trusty_app->props.uuid);

	manifest_data += sizeof(trusty_app->props.uuid);

	memcpy(trusty_app->app_name, manifest_data, 256);

	manifest_data += 256;

	config_blob = (u_int *)manifest_data;
	config_blob_size = (shdr->sh_size - sizeof(uuid_t) - 256);

	trusty_app->props.config_entry_cnt = config_blob_size / sizeof (u_int);

	/* if no config options we're done */
	if (trusty_app->props.config_entry_cnt == 0) {
		return;
	}

	/* save off configuration blob start so it can be accessed later */
	trusty_app->props.config_blob = config_blob;

	/*
	 * Step thru configuration blob.
	 *
	 * Save off some configuration data while we are here but
	 * defer processing of other data until it is needed later.
	 */
	for (i = 0; i < trusty_app->props.config_entry_cnt; i++) {
		switch (config_blob[i]) {
		case TRUSTY_APP_CONFIG_KEY_MIN_STACK_SIZE:
			/* MIN_STACK_SIZE takes 1 data value */
			ASSERT((trusty_app->props.config_entry_cnt - i) > 1);
			trusty_app->props.min_stack_size =
				ROUNDUP(config_blob[++i], 4096);
			ASSERT(trusty_app->props.min_stack_size > 0);
			break;
		case TRUSTY_APP_CONFIG_KEY_MIN_HEAP_SIZE:
			/* MIN_HEAP_SIZE takes 1 data value */
			ASSERT((trusty_app->props.config_entry_cnt - i) > 1);
			trusty_app->props.min_heap_size =
				ROUNDUP(config_blob[++i], 4096);
			ASSERT(trusty_app->props.min_heap_size > 0);
			break;
		case TRUSTY_APP_CONFIG_KEY_MAP_MEM:
			/* MAP_MEM takes 3 data values */
			ASSERT((trusty_app->props.config_entry_cnt - i) > 3);
			trusty_app->props.map_io_mem_cnt++;
			i += 3;
			break;
		default:
			dprintf(CRITICAL,
				"%s: unknown config key: %d\n",
				__func__, config_blob[i]);
			ASSERT(0);
			i++;
			break;
		}
	}

#if DEBUG_LOAD_TRUSTY_APP
	dprintf(SPEW, "trusty_app %p: stack_sz=0x%x\n", trusty_app,
		trusty_app->props.min_stack_size);
	dprintf(SPEW, "trusty_app %p: heap_sz=0x%x\n", trusty_app,
		trusty_app->props.min_heap_size);
	dprintf(SPEW, "trusty_app %p: num_io_mem=%d\n", trusty_app,
		trusty_app->props.map_io_mem_cnt);
#endif
}

static status_t init_brk(trusty_app_t *trusty_app)
{
	status_t status;
	vaddr_t vaddr;
	void *heap;

	trusty_app->cur_brk = trusty_app->start_brk;

	/* do we need to increase user mode heap (if not enough remains)? */
	if ((trusty_app->end_brk - trusty_app->start_brk) >=
	    trusty_app->props.min_heap_size)
		return NO_ERROR;

	heap = memalign(PAGE_SIZE, trusty_app->props.min_heap_size);
	if (heap == 0)
		return ERR_NO_MEMORY;
	memset(heap, 0, trusty_app->props.min_heap_size);

	vaddr = trusty_app->end_brk;
	status = uthread_map_contig(trusty_app->ut, &vaddr,
			     vaddr_to_paddr(heap),
			     trusty_app->props.min_heap_size,
			     UTM_W | UTM_R | UTM_FIXED,
			     UT_MAP_ALIGN_4KB);
	if (status != NO_ERROR || vaddr != trusty_app->end_brk) {
		dprintf(CRITICAL, "cannot map brk\n");
		free(heap);
		return ERR_NO_MEMORY;
	}

	trusty_app->end_brk += trusty_app->props.min_heap_size;
	return NO_ERROR;
}

static status_t alloc_address_map(trusty_app_t *trusty_app)
{
	Elf32_Ehdr *elf_hdr = trusty_app->app_img;
	void *trusty_app_image;
	Elf32_Phdr *prg_hdr;
	u_int i, trusty_app_idx;
	status_t ret;
	vaddr_t start_code = ~0;
	vaddr_t start_data = 0;
	vaddr_t end_code = 0;
	vaddr_t end_data = 0;

	trusty_app_image = trusty_app->app_img;
	trusty_app_idx = trusty_app - trusty_app_list;

	/* create mappings for PT_LOAD sections */
	for (i = 0; i < elf_hdr->e_phnum; i++) {
		vaddr_t first, last, last_mem;

		prg_hdr = (Elf32_Phdr *)(trusty_app_image + elf_hdr->e_phoff +
				(i * sizeof(Elf32_Phdr)));

#if DEBUG_LOAD_TRUSTY_APP
		dprintf(SPEW,
			"trusty_app %d: ELF type 0x%x, vaddr 0x%08x, paddr 0x%08x"
			" rsize 0x%08x, msize 0x%08x, flags 0x%08x\n",
			trusty_app_idx, prg_hdr->p_type, prg_hdr->p_vaddr,
			prg_hdr->p_paddr, prg_hdr->p_filesz, prg_hdr->p_memsz,
			prg_hdr->p_flags);
#endif

		if (prg_hdr->p_type != PT_LOAD)
			continue;

		/* skip PT_LOAD if it's below trusty_app start or above .bss */
		if ((prg_hdr->p_vaddr < TRUSTY_APP_START_ADDR) ||
		    (prg_hdr->p_vaddr >= trusty_app->end_bss))
			continue;

		/*
		 * We're expecting to be able to execute the trusty_app in-place,
		 * meaning its PT_LOAD segments, should be page-aligned.
		 */
		ASSERT(!(prg_hdr->p_vaddr & PAGE_MASK) &&
		       !(prg_hdr->p_offset & PAGE_MASK));

		size_t size = (prg_hdr->p_memsz + PAGE_MASK) & ~PAGE_MASK;
		paddr_t paddr = vaddr_to_paddr(trusty_app_image + prg_hdr->p_offset);
		vaddr_t vaddr = prg_hdr->p_vaddr;
		u_int flags = PF_TO_UTM_FLAGS(prg_hdr->p_flags) | UTM_FIXED;

		ret = uthread_map_contig(trusty_app->ut, &vaddr, paddr, size,
				flags, UT_MAP_ALIGN_4KB);
		if (ret) {
			dprintf(CRITICAL, "cannot map the segment\n");
			return ret;
		}

		vaddr_t stack_bot = TRUSTY_APP_STACK_TOP - trusty_app->props.min_stack_size;
		/* check for overlap into user stack range */
		if (stack_bot < vaddr + size) {
			dprintf(CRITICAL,
				"failed to load trusty_app: (overlaps user stack 0x%lx)\n",
				 stack_bot);
			return ERR_TOO_BIG;
		}

#if DEBUG_LOAD_TRUSTY_APP
		dprintf(SPEW,
			"trusty_app %d: load vaddr 0x%08lx, paddr 0x%08lx,"
			" rsize 0x%08x, msize 0x%08x, access %c%c%c,"
			" flags 0x%x\n",
			trusty_app_idx, vaddr, paddr, size, prg_hdr->p_memsz,
			flags & UTM_R ? 'r' : '-', flags & UTM_W ? 'w' : '-',
			flags & UTM_X ? 'x' : '-', flags);
#endif

		/* start of code/data */
		first = prg_hdr->p_vaddr;
		if (first < start_code)
			start_code = first;
		if (start_data < first)
			start_data = first;

		/* end of code/data */
		last = prg_hdr->p_vaddr + prg_hdr->p_filesz;
		if ((prg_hdr->p_flags & PF_X) && end_code < last)
			end_code = last;
		if (end_data < last)
			end_data = last;

		/* end of brk */
		last_mem = prg_hdr->p_vaddr + prg_hdr->p_memsz;
		if (last_mem > trusty_app->start_brk) {
			void *segment_start = trusty_app_image + prg_hdr->p_offset;

			trusty_app->start_brk = last_mem;
			/* make brk consume the rest of the page */
			trusty_app->end_brk = prg_hdr->p_vaddr + size;

			/* zero fill the remainder of the page for brk.
			 * do it here (instead of init_brk) so we don't
			 * have to keep track of the kernel address of
			 * the mapping where brk starts */
			memset(segment_start + prg_hdr->p_memsz, 0,
			       size - prg_hdr->p_memsz);
		}
	}

	ret = init_brk(trusty_app);
	if (ret != NO_ERROR) {
		dprintf(CRITICAL, "failed to load trusty_app: trusty_app heap creation error\n");
		return ret;
	}

	dprintf(SPEW, "trusty_app %d: code: start 0x%08lx end 0x%08lx\n",
		trusty_app_idx, start_code, end_code);
	dprintf(SPEW, "trusty_app %d: data: start 0x%08lx end 0x%08lx\n",
		trusty_app_idx, start_data, end_data);
	dprintf(SPEW, "trusty_app %d: bss:                end 0x%08lx\n",
		trusty_app_idx, trusty_app->end_bss);
	dprintf(SPEW, "trusty_app %d: brk:  start 0x%08lx end 0x%08lx\n",
		trusty_app_idx, trusty_app->start_brk, trusty_app->end_brk);

	dprintf(SPEW, "trusty_app %d: entry 0x%08lx\n", trusty_app_idx, trusty_app->ut->entry);

	return NO_ERROR;
}

/*
 * Align the next trusty_app to a page boundary, by copying what remains
 * in the trusty_app image to the aligned next trusty_app start. This should be
 * called after we're done with the section headers as the previous
 * trusty_apps .shstrtab section will be clobbered.
 *
 * Note: trusty_app_image_size remains the carved out part in LK to exit
 * the bootloader loop, so still increment by max_extent. Because of
 * the copy down to an aligned next trusty_app addr, trusty_app_image_size is
 * more than what we're actually using.
 */
static char *align_next_app(Elf32_Ehdr *elf_hdr, Elf32_Shdr *pad_hdr,
			    u_int max_extent)
{
	char *next_trusty_app_align_start;
	char *next_trusty_app_fsize_start;
	char *trusty_app_image_addr;
	u_int copy_size;

	ASSERT(ROUNDUP(max_extent, 4) == elf_hdr->e_shoff);
	ASSERT(pad_hdr);

	trusty_app_image_addr = (char *)elf_hdr;
	max_extent = (elf_hdr->e_shoff + (elf_hdr->e_shnum * elf_hdr->e_shentsize)) - 1;
	ASSERT((trusty_app_image_addr + max_extent + 1) <= trusty_app_image_end);

	next_trusty_app_align_start = trusty_app_image_addr + pad_hdr->sh_offset + pad_hdr->sh_size;
	next_trusty_app_fsize_start = trusty_app_image_addr + max_extent + 1;
	ASSERT(next_trusty_app_align_start <= next_trusty_app_fsize_start);

	copy_size = trusty_app_image_end - next_trusty_app_fsize_start;
	if (copy_size) {
		/*
		 * Copy remaining image bytes to aligned start for the next
		 * (and subsequent) trusty_apps. Also decrement trusty_app_image_end, so
		 * we copy less each time we realign for the next trusty_app.
		 */
		memcpy(next_trusty_app_align_start, next_trusty_app_fsize_start, copy_size);
		arch_sync_cache_range((addr_t)next_trusty_app_align_start,
				       copy_size);
		trusty_app_image_end -= (next_trusty_app_fsize_start - next_trusty_app_align_start);
	}

	trusty_app_image_size -= (max_extent + 1);
	return next_trusty_app_align_start;
}

/*
 * Look in the kernel's ELF header for trusty_app sections and
 * carveout memory for their LOAD-able sections.
 */
static void trusty_app_bootloader(void)
{
	Elf32_Ehdr *ehdr;
	Elf32_Shdr *shdr;
	Elf32_Shdr *bss_shdr, *bss_pad_shdr, *manifest_shdr;
	char *shstbl, *trusty_app_image_addr;
	trusty_app_t *trusty_app = 0;

	dprintf(SPEW, "trusty_app: start %p size 0x%08x end %p\n",
		trusty_app_image_start, trusty_app_image_size, trusty_app_image_end);

	trusty_app_image_addr = trusty_app_image_start;

	/* alloc trusty_app_list page from carveout */
	if (trusty_app_image_size) {
		trusty_app_list = (trusty_app_t *)memalign(PAGE_SIZE, PAGE_SIZE);
		trusty_app = trusty_app_list;
		memset(trusty_app, 0, PAGE_SIZE);
	}

	while (trusty_app_image_size > 0) {
		u_int i, trusty_app_max_extent;

		ehdr = (Elf32_Ehdr *) trusty_app_image_addr;
		if (strncmp((char *)ehdr->e_ident, ELFMAG, SELFMAG)) {
			dprintf(CRITICAL, "trusty_app_bootloader: ELF header not found\n");
			break;
		}

		shdr = (Elf32_Shdr *) ((intptr_t)ehdr + ehdr->e_shoff);
		shstbl = (char *)((intptr_t)ehdr + shdr[ehdr->e_shstrndx].sh_offset);

		trusty_app_max_extent = 0;
		bss_shdr = bss_pad_shdr = manifest_shdr = NULL;

		/* calculate trusty_app end */
		for (i = 0; i < ehdr->e_shnum; i++) {
			u_int extent;

			if (shdr[i].sh_type == SHT_NULL)
				continue;
#if DEBUG_LOAD_TRUSTY_APP
			dprintf(SPEW, "trusty_app: sect %d, off 0x%08x, size 0x%08x, flags 0x%02x, name %s\n",
				i, shdr[i].sh_offset, shdr[i].sh_size, shdr[i].sh_flags, shstbl + shdr[i].sh_name);
#endif

			/* track bss and manifest sections */
			if (!strcmp((shstbl + shdr[i].sh_name), ".bss")) {
				bss_shdr = shdr + i;
				trusty_app->end_bss = bss_shdr->sh_addr + bss_shdr->sh_size;
			}
			else if (!strcmp((shstbl + shdr[i].sh_name), ".bss-pad")) {
				bss_pad_shdr = shdr + i;
			}
			else if (!strcmp((shstbl + shdr[i].sh_name),
					 ".trusty_app.manifest")) {
				manifest_shdr = shdr + i;
			}

			if (shdr[i].sh_type != SHT_NOBITS) {
				extent = shdr[i].sh_offset + shdr[i].sh_size;
				if (trusty_app_max_extent < extent)
					trusty_app_max_extent = extent;
			}
		}

		/* we need these sections */
		ASSERT(bss_shdr && bss_pad_shdr && manifest_shdr);

		/* clear .bss */
		ASSERT((bss_shdr->sh_offset + bss_shdr->sh_size) <= trusty_app_max_extent);
		memset((uint8_t *)trusty_app_image_addr + bss_shdr->sh_offset, 0, bss_shdr->sh_size);

		load_app_config_options((intptr_t)trusty_app_image_addr, trusty_app, manifest_shdr);
		trusty_app->app_img = ehdr;

		/* align next trusty_app start */
		trusty_app_image_addr = align_next_app(ehdr, bss_pad_shdr, trusty_app_max_extent);

		ASSERT((trusty_app_count + 1) < MAX_TRUSTY_APP_COUNT);
		trusty_app_count++;
		trusty_app++;
	}
}

status_t trusty_app_setup_mmio(trusty_app_t *trusty_app, u_int mmio_id,
		vaddr_t *vaddr, uint32_t map_size)
{
	u_int i;
	u_int id, offset, size;

	/* step thru configuration blob looking for I/O mapping requests */
	for (i = 0; i < trusty_app->props.config_entry_cnt; i++) {
		if (trusty_app->props.config_blob[i] == TRUSTY_APP_CONFIG_KEY_MAP_MEM) {
			id = trusty_app->props.config_blob[++i];
			offset = trusty_app->props.config_blob[++i];
			size = ROUNDUP(trusty_app->props.config_blob[++i],
					PAGE_SIZE);

			if (id != mmio_id)
				continue;

			map_size = ROUNDUP(map_size, PAGE_SIZE);
			if (map_size > size)
				return ERR_INVALID_ARGS;

			return uthread_map_contig(trusty_app->ut, vaddr, offset,
						map_size, UTM_W | UTM_R | UTM_IO,
						UT_MAP_ALIGN_4KB);
		} else {
			/* all other config options take 1 data value */
			i++;
		}
	}

	return ERR_NOT_FOUND;
}

void trusty_app_init(void)
{
	trusty_app_t *trusty_app;
	u_int i;

	trusty_app_image_start = (char *)&__trusty_app_start;
	trusty_app_image_end = (char *)&__trusty_app_end;
	trusty_app_image_size = (trusty_app_image_end - trusty_app_image_start);

	ASSERT(!((uintptr_t)trusty_app_image_start & PAGE_MASK));

	finalize_registration();

	trusty_app_bootloader();

	for (i = 0, trusty_app = trusty_app_list; i < trusty_app_count; i++, trusty_app++) {
		char name[32];
		uthread_t *uthread;
		int ret;

		snprintf(name, sizeof(name), "%s, trusty_app_%d_%08x-%04x-%04x",
			 trusty_app->app_name,
			 i,
			 trusty_app->props.uuid.time_low,
			 trusty_app->props.uuid.time_mid,
			 trusty_app->props.uuid.time_hi_and_version);

		/* entry is 0 at this point since we haven't parsed the elf hdrs
		 * yet */
		Elf32_Ehdr *elf_hdr = trusty_app->app_img;
		uthread = uthread_create(name, elf_hdr->e_entry,
					 DEFAULT_PRIORITY, TRUSTY_APP_STACK_TOP,
					 trusty_app->props.min_stack_size, trusty_app);
		if (uthread == NULL) {
			/* TODO: do better than this */
			panic("allocate user thread failed\n");
		}
		trusty_app->ut = uthread;

		ret = alloc_address_map(trusty_app);
		if (ret != NO_ERROR) {
			panic("failed to load address map\n");
		}

		/* attach als_cnt */
		trusty_app->als = calloc(1, als_slot_cnt * sizeof(void*));
		if (!trusty_app->als) {
			panic("allocate app local storage failed\n");
		}

		/* call all registered startup notifiers */
		trusty_app_notifier_t *n;
		list_for_every_entry(&app_notifier_list, n, trusty_app_notifier_t, node) {
			if (n->startup) {
				ret = n->startup(trusty_app);
				if (ret != NO_ERROR)
					panic("failed (%d) to invoke startup notifier\n", ret);
			}
		}
	}
}

trusty_app_t *trusty_app_find_by_uuid(uuid_t *uuid)
{
	trusty_app_t *ta;
	u_int i;

	/* find app for this uuid */
	for (i = 0, ta = trusty_app_list; i < trusty_app_count; i++, ta++)
		if (!memcmp(&ta->props.uuid, uuid, sizeof(uuid_t)))
			return ta;

	return NULL;
}

/* rather export trusty_app_list?  */
void trusty_app_forall(void (*fn)(trusty_app_t *ta, void *data), void *data)
{
	u_int i;
	trusty_app_t *ta;

	if (fn == NULL)
		return;

	for (i = 0, ta = trusty_app_list; i < trusty_app_count; i++, ta++)
		fn(ta, data);
}

static void start_apps(uint level)
{
	trusty_app_t *trusty_app;
	u_int i;
	int ret;

	for (i = 0, trusty_app = trusty_app_list; i < trusty_app_count; i++, trusty_app++) {
		if (trusty_app->ut->entry) {
			ret = uthread_start(trusty_app->ut);
			if (ret)
				panic("Cannot start Trusty app!\n");
		}
	}
}

LK_INIT_HOOK(libtrusty_apps, start_apps, LK_INIT_LEVEL_APPS + 1);
