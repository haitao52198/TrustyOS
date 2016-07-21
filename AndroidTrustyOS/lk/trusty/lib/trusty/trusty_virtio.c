/*
 * Copyright (c) 2015, Google, Inc. All rights reserved
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

#include <err.h>
#include <assert.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include <sys/types.h>

#include <lk/init.h>
#include <kernel/vm.h>
#include <kernel/mutex.h>
#include <arch/arch_ops.h>

#include <remoteproc/remoteproc.h>
#include "trusty_virtio.h"

#define LOCAL_TRACE  0

/*
 *  FW resource version expected by us
 */
#define VIRTIO_FW_RSC_VER  1

typedef struct resource_table trusty_virtio_descr_t;

enum {
	VIRTIO_BUS_STATE_UNINITIALIZED = 0,
	VIRTIO_BUS_STATE_IDLE,
	VIRTIO_BUS_STATE_ACTIVATING,
	VIRTIO_BUS_STATE_ACTIVE,
	VIRTIO_BUS_STATE_DEACTIVATING,
};

struct trusty_virtio_bus {
	uint   vdev_cnt;
	uint   next_dev_id;
	size_t  descr_size;
	volatile int state;
	struct list_node vdev_list;
};


static struct trusty_virtio_bus _virtio_bus = {
	.vdev_cnt = 0,
	.descr_size = 0,
	.next_dev_id = 0,
	.state = VIRTIO_BUS_STATE_UNINITIALIZED,
	.vdev_list = LIST_INITIAL_VALUE(_virtio_bus.vdev_list),
};

static status_t map_descr(ns_paddr_t buf_pa, void **buf_va, ns_size_t sz,
                          uint buf_mmu_flags)
{
	if (!buf_pa) {
		LTRACEF("invalid descr addr 0x%llx\n", buf_pa);
		return ERR_INVALID_ARGS;
	}

	if (buf_pa & (PAGE_SIZE-1)) {
		LTRACEF("unexpected alignment 0x%llx\n", buf_pa);
		return ERR_INVALID_ARGS;
	}

#if !IS_64BIT
	if (buf_pa >> 32) {
		LTRACEF("unsuported addr range 0x%llx\n", buf_pa);
		return ERR_INVALID_ARGS;
	}
#endif

	/* map resource table into our address space */
	return  vmm_alloc_physical(vmm_get_kernel_aspace(), "virtio",
	                           ROUNDUP(sz, PAGE_SIZE),
	                           buf_va, PAGE_SIZE_SHIFT,
	                           (paddr_t) buf_pa, 0, buf_mmu_flags);
}

static status_t unmap_descr(ns_paddr_t pa, void *va, size_t sz)
{
	return vmm_free_region(vmm_get_kernel_aspace(), (vaddr_t)va);
}

static status_t validate_vdev(struct vdev *vd)
{
	/* check parameters */
	if (!vd) {
		LTRACEF("vd = %p\n", vd);
		return ERR_INVALID_ARGS;
	}

	/* check vdev_ops */
	if (!vd->ops) {
		LTRACEF("vd = %p: missing vdev ops\n", vd);
		return ERR_INVALID_ARGS;
	}

	/* all vdev_ops required */
	const struct vdev_ops *ops = vd->ops;
	if (!ops->descr_sz || !ops->get_descr ||
	    !ops->probe || !ops->reset || !ops->kick_vqueue) {
		LTRACEF("vd = %p: misconfigured vdev ops\n", vd);
		return ERR_INVALID_ARGS;
	}

	return NO_ERROR;
}

/*
 *     Register virtio device
 */
status_t virtio_register_device(struct vdev *vd)
{
	status_t ret = ERR_BAD_STATE;
	struct trusty_virtio_bus *vb = &_virtio_bus;

	if (vb->state == VIRTIO_BUS_STATE_UNINITIALIZED) {
		ret = validate_vdev(vd);
		if (ret == NO_ERROR) {
			vb->vdev_cnt++;
			vd->devid = vb->next_dev_id++;
			list_add_tail(&vb->vdev_list, &vd->node);
		}
	}
	return ret;
}

/*
 *
 */
static void finalize_vdev_registery(void)
{
	struct trusty_virtio_bus *vb = &_virtio_bus;

	if (vb->state == VIRTIO_BUS_STATE_UNINITIALIZED) {
		struct vdev *vd;
		uint32_t offset = sizeof(struct resource_table) +
			          sizeof(uint32_t) * vb->vdev_cnt;

		/*
		 * go through the list of vdev and calculate
		 * total descriptor size and offset withing descriptor
		 * buffer
		 */
		list_for_every_entry(&vb->vdev_list, vd, struct vdev, node) {
			vd->descr_offset = offset;
			offset += vd->ops->descr_sz(vd);
		}
		vb->descr_size = offset;
		vb->state = VIRTIO_BUS_STATE_IDLE;
	}
}

/*
 * Retrieve device description to be shared with NS side
 */
ssize_t virtio_get_description(ns_paddr_t buf_pa, ns_size_t buf_sz,
                               uint buf_mmu_flags)
{
	status_t ret;
	struct vdev *vd;
	struct trusty_virtio_bus *vb = &_virtio_bus;

	LTRACEF("descr_buf: %u bytes @ 0x%llx\n", buf_sz, buf_pa);

	finalize_vdev_registery();

	if ((size_t)buf_sz < vb->descr_size) {
		LTRACEF("buffer (%zu bytes) is too small (%zu needed)\n",
			 (size_t)buf_sz, vb->descr_size);
		return ERR_NOT_ENOUGH_BUFFER;
	}

	/* map in NS memory */
	void *va = NULL;
	ret = map_descr(buf_pa, &va, vb->descr_size, buf_mmu_flags);
	if (ret != NO_ERROR) {
		LTRACEF("failed (%d) to map in descriptor buffer\n",
			(int)ret);
		return ret;
	}
	memset(va, 0, vb->descr_size);

	/* build resource table */
	uint32_t vdev_idx = 0;
	trusty_virtio_descr_t *descr = (trusty_virtio_descr_t *)va;

	descr->ver = VIRTIO_FW_RSC_VER;
	descr->num = vb->vdev_cnt;

	list_for_every_entry(&vb->vdev_list, vd, struct vdev, node) {
		DEBUG_ASSERT(vd->descr_offset <= vb->descr_size);
		DEBUG_ASSERT(vd->descr_offset + vd->ops->descr_sz(vd) <= vb->descr_size);
		DEBUG_ASSERT(vdev_idx < vb->vdev_cnt);

		descr->offset[vdev_idx++] = vd->descr_offset;
		vd->ops->get_descr(vd, (uint8_t *)descr + vd->descr_offset);
	}

	unmap_descr(buf_pa, va, vb->descr_size);

	return vb->descr_size;
}

/*
 * Called by NS side to finalize device initialization
 */
status_t virtio_start(ns_paddr_t ns_descr_pa, ns_size_t descr_sz,
                      uint descr_mmu_flags)
{
	status_t ret;
	int oldstate;
	void *descr_va;
	void *ns_descr_va = NULL;
	struct trusty_virtio_bus *vb = &_virtio_bus;

	LTRACEF("%u bytes @ 0x%llx\n", descr_sz, ns_descr_pa);

	oldstate = atomic_cmpxchg(&vb->state,
				  VIRTIO_BUS_STATE_IDLE,
				  VIRTIO_BUS_STATE_ACTIVATING);

	if (oldstate != VIRTIO_BUS_STATE_IDLE) {
		/* bus should be in initializing state */
		LTRACEF("unexpected state state (%d)\n", oldstate);
		return ERR_BAD_STATE;
	}

	if ((size_t)descr_sz != vb->descr_size) {
		LTRACEF("unexpected descriptor size (%zd vs. %zd)\n",
			(size_t)descr_sz, vb->descr_size);
		ret = ERR_INVALID_ARGS;
		goto err_bad_params;
	}

	descr_va = malloc(descr_sz);
	if (!descr_va) {
		LTRACEF("not enough memory to store descr\n");
		ret = ERR_NO_MEMORY;
		goto err_alloc_descr;
	}

	ret = map_descr(ns_descr_pa, &ns_descr_va, vb->descr_size, descr_mmu_flags);
	if (ret != NO_ERROR) {
		LTRACEF("failed (%d) to map in descriptor buffer\n", ret);
		goto err_map_in;
	}

	/* copy descriptor out of NS memory before parsing it */
	memcpy(descr_va, ns_descr_va, descr_sz);

	/* handle it */
	struct vdev *vd;
	list_for_every_entry(&vb->vdev_list, vd, struct vdev, node) {
		vd->ops->probe(vd, (void*)((uint8_t*)descr_va + vd->descr_offset));
	}

	unmap_descr(ns_descr_pa, ns_descr_va, vb->descr_size);
	free(descr_va);

	vb->state = VIRTIO_BUS_STATE_ACTIVE;

	return NO_ERROR;

err_map_in:
	free(descr_va);
err_alloc_descr:
err_bad_params:
	vb->state = oldstate;
	return ret;
}

status_t virtio_stop(ns_paddr_t descr_pa, ns_size_t descr_sz, uint descr_mmu_flags)
{
	int oldstate;
	struct vdev *vd;
	struct trusty_virtio_bus *vb = &_virtio_bus;

	LTRACEF("%u bytes @ 0x%llx\n", descr_sz, descr_pa);

	oldstate = atomic_cmpxchg(&vb->state,
				  VIRTIO_BUS_STATE_ACTIVE,
				  VIRTIO_BUS_STATE_DEACTIVATING);

	if (oldstate != VIRTIO_BUS_STATE_ACTIVE)
		return ERR_BAD_STATE;

	/* reset all devices */
	list_for_every_entry(&vb->vdev_list, vd, struct vdev, node) {
		vd->ops->reset(vd);
	}

	vb->state = VIRTIO_BUS_STATE_IDLE;

	return NO_ERROR;
}

/*
 *  Reset virtio device with specified device id
 */
status_t virtio_device_reset(uint devid)
{
	struct vdev *vd;
	status_t ret = ERR_NOT_FOUND;
	struct trusty_virtio_bus *vb = &_virtio_bus;

	LTRACEF("dev=%d\n", devid);

	if (vb->state != VIRTIO_BUS_STATE_ACTIVE)
		return ERR_BAD_STATE;

	list_for_every_entry(&vb->vdev_list, vd, struct vdev, node) {
		if (vd->devid == devid) {
			ret = vd->ops->reset(vd);
			break;
		}
	}
	return ret;
}

/*
 *  Kick vq for virtio device with specified device id
 */
status_t virtio_kick_vq(uint devid, uint vqid)
{
	struct vdev *vd;
	status_t ret = ERR_NOT_FOUND;
	struct trusty_virtio_bus *vb = &_virtio_bus;

#if WITH_CHATTY_LTRACE
	LTRACEF("dev=%d\n", devid);
#endif

	if (vb->state != VIRTIO_BUS_STATE_ACTIVE)
		return ERR_BAD_STATE;

	list_for_every_entry(&vb->vdev_list, vd, struct vdev, node) {
		if (vd->devid == devid) {
			ret = vd->ops->kick_vqueue(vd, vqid);
			break;
		}
	}
	return ret;
}

