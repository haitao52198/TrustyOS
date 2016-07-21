/*
 * Copyright (c) 2014-2015, Google, Inc. All rights reserved
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
#include <arch/defines.h>
#include <remoteproc/remoteproc.h>
#include <lib/trusty/uuid.h>

struct tipc_dev;

/*
 * This ID has to match to the value defined in virtio_ids.h on Linux side
 */
#define VIRTIO_ID_TIPC			(13)

/*
 * TIPC device supports 2 vqueues: TX and RX
 */
#define TIPC_VQ_TX			(0)
#define TIPC_VQ_RX			(1)
#define TIPC_VQ_NUM			(2)

/*
 *  Maximum device name size
 */
#define TIPC_MAX_DEV_NAME_LEN		(32)

/*
 *  Trusty IPC device configuration shared with linux side
 */
struct tipc_dev_config {
	uint32_t msg_buf_max_size;  /* max msg size that this device can handle */
	uint32_t msg_buf_alignment; /* required msg alignment (PAGE_SIZE) */
	char     dev_name[TIPC_MAX_DEV_NAME_LEN]; /* NS device node name  */
} __PACKED;

struct tipc_vdev_descr {
	struct fw_rsc_hdr		hdr;
	struct fw_rsc_vdev		vdev;
	struct fw_rsc_vdev_vring	vrings[TIPC_VQ_NUM];
	struct tipc_dev_config		config;
} __PACKED;


#define DECLARE_TIPC_DEVICE_DESCR(_nm, _nid, _txvq_sz, _rxvq_sz, _nd_name) \
static const struct tipc_vdev_descr _nm = {                          \
	.hdr.type	= RSC_VDEV,                                  \
	.vdev		= {                                          \
		.id		= VIRTIO_ID_TIPC,                    \
		.notifyid	= _nid,                              \
		.dfeatures	= 0,                                 \
		.config_len	= sizeof(struct tipc_dev_config),    \
		.num_of_vrings	= TIPC_VQ_NUM,                       \
	},                                                           \
	.vrings	= {                                                  \
		[TIPC_VQ_TX]	= {                                  \
			.align		= PAGE_SIZE,                 \
			.num		= (_txvq_sz),                \
			.notifyid	= 1,                         \
		},                                                   \
		[TIPC_VQ_RX]	= {                                  \
			.align		= PAGE_SIZE,                 \
			.num		= (_rxvq_sz),                \
			.notifyid	= 2,                         \
		},                                                   \
	},                                                           \
	.config = {                                                  \
		.msg_buf_max_size  = PAGE_SIZE,                      \
		.msg_buf_alignment = PAGE_SIZE,                      \
		.dev_name = _nd_name                                 \
	}                                                            \
};                                                                   \


/*
 *  Create TIPC device and register it witth virtio subsystem
 */
status_t create_tipc_device(const struct tipc_vdev_descr *descr, size_t descr_sz,
                            const uuid_t *uuid, struct tipc_dev **dev_ptr);


/*
 *  Check if uuid belongs to NS client
 */
bool is_ns_client(const uuid_t *uuid);

