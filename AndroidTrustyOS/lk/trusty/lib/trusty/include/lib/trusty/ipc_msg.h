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

#ifndef __LIB_TRUSTY_IPC_MSG_H
#define __LIB_TRUSTY_IPC_MSG_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <uthread.h>

#include <lib/trusty/uctx.h>
#include <lib/trusty/uio.h>

typedef struct ipc_msg_queue ipc_msg_queue_t;

int ipc_msg_queue_create(uint num_items, size_t item_sz, ipc_msg_queue_t **mq);
void ipc_msg_queue_destroy(ipc_msg_queue_t *mq);

bool ipc_msg_queue_is_empty(ipc_msg_queue_t *mq);
bool ipc_msg_queue_is_full(ipc_msg_queue_t *mq);

/********** these structure definitions shared with userspace **********/

/* The layout for iovec_user and ipc_msg_user MUST match
 * the layout of iovec_kern and ipc_msg_kern
 */
#define MAX_MSG_HANDLES	8

typedef struct ipc_msg_kern {
	uint		num_iov;
	iovec_kern_t	*iov;

	uint		num_handles;
	handle_id_t	*handles;
} ipc_msg_kern_t;

typedef struct ipc_msg_user {
	uint32_t	num_iov;
	user_addr_t	iov;

	uint32_t	num_handles;
	user_addr_t	handles;
} ipc_msg_user_t;

typedef struct ipc_msg_info {
	uint32_t	len;
	uint32_t	id;
} ipc_msg_info_t;

int ipc_get_msg(handle_t *chandle, ipc_msg_info_t *msg_info);
int ipc_read_msg(handle_t *chandle, uint32_t msg_id, uint32_t offset,
		 ipc_msg_kern_t *msg);
int ipc_put_msg(handle_t *chandle, uint32_t msg_id);
int ipc_send_msg(handle_t *chandle, ipc_msg_kern_t *msg);

#endif
