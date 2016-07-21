
/*
 * Copyright (c) 2014, Google, Inc. All rights reserved
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

#pragma once

#include <sys/types.h>
#include <list.h>
#include <lib/sm.h>

__BEGIN_CDECLS

enum {
	VDEV_STATE_RESET = 0,
	VDEV_STATE_ACTIVE,
};

struct vdev;

struct vdev_ops {
	size_t   (*descr_sz)(struct vdev *vd);
	ssize_t  (*get_descr)(struct vdev *vd, void *descr);
	status_t (*probe)(struct vdev *vd, void *descr);
	status_t (*reset)(struct vdev *vd);
	status_t (*kick_vqueue)(struct vdev *vd, uint vqid);
};

struct vdev {
	volatile int state;
	uint   devid;
	uint   descr_offset;
	struct list_node node;
	const struct vdev_ops *ops;
};

typedef uint64_t ns_paddr_t;

/*
 * Register virtio device
 */
status_t virtio_register_device(struct vdev *vd);

/*
 * Retrieve device description to be shared with NS side
 */
ssize_t virtio_get_description(ns_paddr_t descr_pa, ns_size_t sz, uint mmu_flags);

/*
 * Called by NS side to finilize initialization
 */
status_t virtio_start(ns_paddr_t descr_pa, ns_size_t sz, uint mmu_flags);

/*
 * Called by NS side to deinitialize virtio subsystem
 */
status_t virtio_stop(ns_paddr_t descr_pa, ns_size_t sz, uint mmu_flags);

/*
 *  Reset virtio device with specified device id
 */
status_t virtio_device_reset(uint devid);

/*
 *  Kick vq for specified device
 */
status_t virtio_kick_vq(uint devid, uint vqid);


__END_CDECLS


