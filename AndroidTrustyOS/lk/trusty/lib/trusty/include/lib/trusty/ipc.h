/*
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

#ifndef __LIB_TRUSTY_IPC_H
#define __LIB_TRUSTY_IPC_H

#include <bits.h>
#include <kernel/mutex.h>
#include <kernel/thread.h>
#include <reflist.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

#include <lib/trusty/handle.h>
#include <lib/trusty/ipc_msg.h>
#include <lib/trusty/uuid.h>

enum {
	IPC_PORT_STATE_INVALID		= 0,
	IPC_PORT_STATE_LISTENING	= 1,
	IPC_PORT_STATE_CLOSING		= 2,
};

enum {
	IPC_PORT_ALLOW_TA_CONNECT	= 0x1,
	IPC_PORT_ALLOW_NS_CONNECT	= 0x2,
};

#define IPC_PORT_PATH_MAX	64

typedef struct ipc_port {
	/* e.g. /service/sys/crypto, /service/usr/drm/widevine */
	char			path[IPC_PORT_PATH_MAX];
	const struct uuid	*uuid;

	uint32_t		state;
	uint32_t		flags;

	uint			num_recv_bufs;
	size_t			recv_buf_size;

	handle_t		handle;

	struct list_node	pending_list;

	struct list_node	node;
} ipc_port_t;

enum {
	IPC_CHAN_STATE_INVALID		= 0,
	IPC_CHAN_STATE_ACCEPTING	= 1,
	IPC_CHAN_STATE_CONNECTING	= 2,
	IPC_CHAN_STATE_CONNECTED	= 3,
	IPC_CHAN_STATE_DISCONNECTING	= 4,
	IPC_CHAN_STATE_WAITING_FOR_PORT	= 5,
};

enum {
	IPC_CHAN_FLAG_SERVER		= 0x1,
};

/* aux state bitmasks */
enum {
	IPC_CHAN_AUX_STATE_SEND_BLOCKED = 0x1,
	IPC_CHAN_AUX_STATE_SEND_UNBLOCKED = 0x2,
	IPC_CHAN_AUX_STATE_CONNECTED = 0x4,
};

#define IPC_CHAN_MAX_BUFS	32
#define IPC_CHAN_MAX_BUF_SIZE	4096

typedef struct ipc_chan {
	obj_t			refobj;
	obj_ref_t		peer_ref;
	struct ipc_chan		*peer;
	const struct uuid	*uuid;

	uint32_t		state;
	uint32_t		flags;
	uint32_t		aux_state;

	/* handle_ref is a self reference when there are
	 * outstanding handles out there. It is removed
	 * when last handle ref goes away.
	 */
	obj_ref_t		handle_ref;
	handle_t		handle;

	/* used for port's pending list. node_ref field is a
	 * self reference when node field is inserted in the list.
	 *
	 * TODO: consider creating generic solution by grouping
	 * together list_node and obj_ref_t into single struct.
	 */
	obj_ref_t		node_ref;
	struct list_node	node;

	ipc_msg_queue_t		*msg_queue;

	/*
	 * TODO: consider changing async connect to preallocate
	 *       not-yet-existing port object then we can get rid
	  *      of this field.
	 */
	const char		*path;
} ipc_chan_t;

/* called by server to create port */
int ipc_port_create(const uuid_t *sid, const char *path,
                    uint num_recv_bufs, size_t recv_buf_size,
                    uint32_t flags,  handle_t **phandle_ptr);

/* called by server to publish the port */
int ipc_port_publish(handle_t *phandle);

/* server calls to accept a pending connection */
int ipc_port_accept(handle_t *phandle, handle_t **chandle_ptr,
                    const uuid_t **uuid_ptr);

/* client requests a connection to a port */
enum {
	IPC_CONNECT_WAIT_FOR_PORT = 0x1,
	IPC_CONNECT_ASYNC = 0x2,
	IPC_CONNECT_MASK = IPC_CONNECT_WAIT_FOR_PORT
			 | IPC_CONNECT_ASYNC,
};
int ipc_port_connect_async(const uuid_t *cid, const char *path, size_t max_path,
			   uint flags, handle_t **chandle_ptr);

bool ipc_is_channel(handle_t *handle);
bool ipc_is_port(handle_t *handle);

#endif
