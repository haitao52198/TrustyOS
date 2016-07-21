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

#define LOCAL_TRACE 0

/**
 * @file
 * @brief  IPC message management primitives
 * @defgroup ipc IPC
 *
 * Provides low level data structures for managing message
 * areas for the ipc contexts.
 *
 * Also provides user syscall implementations for message
 * send/receive mechanism.
 *
 * @{
 */

#include <assert.h>
#include <err.h>
#include <list.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <trace.h>
#include <uthread.h>

#include <lib/syscall.h>

#if WITH_TRUSTY_IPC

#include <lib/trusty/handle.h>
#include <lib/trusty/ipc.h>
#include <lib/trusty/ipc_msg.h>
#include <lib/trusty/trusty_app.h>
#include <lib/trusty/uctx.h>

extern mutex_t ipc_lock;

enum {
	MSG_ITEM_STATE_FREE	= 0,
	MSG_ITEM_STATE_FILLED	= 1,
	MSG_ITEM_STATE_READ	= 2,
};

typedef struct msg_item {
	uint8_t			id;
	uint8_t			state;
	uint			num_handles;
	handle_id_t		handles[MAX_MSG_HANDLES];
	size_t			len;
	struct list_node	node;
} msg_item_t;

typedef struct ipc_msg_queue {
	struct list_node	free_list;
	struct list_node	filled_list;
	struct list_node	read_list;

	uint			num_items;
	size_t			item_sz;

	uint8_t			*buf;

	/* store the message descriptors in the queue,
	 * and the buffer separately. The buffer could
	 * eventually move to a separate area that can
	 * be mapped into the process directly.
	 */
	msg_item_t		items[0];
} ipc_msg_queue_t;

enum {
	IPC_MSG_BUFFER_USER	= 0,
	IPC_MSG_BUFFER_KERNEL	= 1,
};

typedef struct msg_desc {
	int type;
	union {
		ipc_msg_kern_t	kern;
		ipc_msg_user_t	user;
	};
} msg_desc_t;

/**
 * @brief  Create IPC message queue
 *
 * Stores up-to a predefined number of equal-sized items in a circular
 * buffer (FIFO).
 *
 * @param num_items   Number of messages we need to store.
 * @param item_sz     Size of each message item.
 * @param mq          Pointer where to store the ptr to the newly allocated
 *                    message queue.
 *
 * @return  Returns NO_ERROR on success, ERR_NO_MEMORY on error.
 */
int ipc_msg_queue_create(uint num_items, size_t item_sz, ipc_msg_queue_t **mq)
{
	ipc_msg_queue_t *tmp_mq;
	int ret;

	tmp_mq = calloc(1, (sizeof(ipc_msg_queue_t) +
			    num_items * sizeof(msg_item_t)));
	if (!tmp_mq) {
		dprintf(CRITICAL, "cannot allocate memory for message queue\n");
		return ERR_NO_MEMORY;
	}

	tmp_mq->buf = malloc(num_items * item_sz);
	if (!tmp_mq->buf) {
		dprintf(CRITICAL,
			"cannot allocate memory for message queue buf\n");
		ret = ERR_NO_MEMORY;
		goto err_alloc_buf;
	}

	tmp_mq->num_items = num_items;
	tmp_mq->item_sz = item_sz;
	list_initialize(&tmp_mq->free_list);
	list_initialize(&tmp_mq->filled_list);
	list_initialize(&tmp_mq->read_list);

	for (uint i = 0; i < num_items; i++) {
		tmp_mq->items[i].id = i;
		list_add_tail(&tmp_mq->free_list, &tmp_mq->items[i].node);
	}
	*mq = tmp_mq;
	return 0;

err_alloc_buf:
	free(tmp_mq);
	return ret;
}

void ipc_msg_queue_destroy(ipc_msg_queue_t *mq)
{
	free(mq->buf);
	free(mq);
}

bool ipc_msg_queue_is_empty(ipc_msg_queue_t *mq)
{
	return list_is_empty(&mq->filled_list);
}

bool ipc_msg_queue_is_full(ipc_msg_queue_t *mq)
{
	return list_is_empty(&mq->free_list);
}

static inline uint8_t *msg_queue_get_buf(ipc_msg_queue_t *mq, msg_item_t *item)
{
	return mq->buf + item->id * mq->item_sz;
}

static inline msg_item_t *msg_queue_get_item(ipc_msg_queue_t *mq, uint32_t id)
{
	return id < mq->num_items ? &mq->items[id] : NULL;
}

static int check_channel_locked(handle_t *chandle)
{
	if (unlikely(!chandle))
		return ERR_INVALID_ARGS;

	if (unlikely(!ipc_is_channel(chandle)))
		return ERR_INVALID_ARGS;

	return NO_ERROR;
}

static int check_channel_connected_locked(handle_t *chandle)
{
	int ret = check_channel_locked(chandle);
	if (unlikely(ret != NO_ERROR))
		return ret;

	ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);

	if (likely(chan->state == IPC_CHAN_STATE_CONNECTED)) {
		DEBUG_ASSERT(chan->peer); /* there should be peer */
		return NO_ERROR;
	}

	if (likely(chan->state == IPC_CHAN_STATE_DISCONNECTING))
		return ERR_CHANNEL_CLOSED;
	else
		return ERR_NOT_READY;
}

static int msg_write_locked(ipc_chan_t *chan, msg_desc_t *msg)
{
	ssize_t ret;
	msg_item_t *item;
	ipc_msg_queue_t *mq = chan->peer->msg_queue;

	item = list_peek_head_type(&mq->free_list, msg_item_t, node);
	if (item == NULL) {
		chan->aux_state |= IPC_CHAN_AUX_STATE_SEND_BLOCKED;
		return ERR_NOT_ENOUGH_BUFFER;
	}

	DEBUG_ASSERT(item->state == MSG_ITEM_STATE_FREE);

	/* TODO: figure out what to do about handles */
	item->num_handles = 0;
	item->len = 0;

	uint8_t *buf = msg_queue_get_buf(mq, item);

	if (msg->type == IPC_MSG_BUFFER_KERNEL) {
		if (msg->kern.num_handles) {
			LTRACEF("handles are not supported yet\n");
			return ERR_NOT_SUPPORTED;
		}
		ret = kern_iovec_to_membuf(buf, mq->item_sz,
		                          (const iovec_kern_t *)msg->kern.iov,
		                           msg->kern.num_iov);
	} else if (msg->type == IPC_MSG_BUFFER_USER) {
		if (msg->user.num_handles) {
			LTRACEF("handles are not supported yet\n");
			return ERR_NOT_SUPPORTED;
		}
		ret = user_iovec_to_membuf(buf, mq->item_sz,
		                           msg->user.iov, msg->user.num_iov);
	} else {
		return ERR_INVALID_ARGS;
	}

	if (ret < 0)
		return ret;

	item->len = (size_t) ret;
	list_delete(&item->node);
	list_add_tail(&mq->filled_list, &item->node);
	item->state = MSG_ITEM_STATE_FILLED;

	return item->len;
}

/*
 * reads the specified message by copying the data into the iov list
 * provided by msg. The message must have been previously moved
 * to the read list (and thus put into READ state).
 */
static int msg_read_locked(ipc_msg_queue_t *mq, uint32_t msg_id,
                           uint32_t offset, msg_desc_t *msg)
{
	msg_item_t *item;

	item = msg_queue_get_item(mq, msg_id);
	if (!item) {
		LTRACEF("invalid message id %d\n", msg_id);
		return ERR_INVALID_ARGS;
	}

	if (item->state != MSG_ITEM_STATE_READ) {
		LTRACEF("message %d is not in READ state (0x%x)\n",
		         item->id, item->state);
		return ERR_INVALID_ARGS;
	}

	if (item->num_handles) {
		LTRACEF("handles are not supported yet\n");
		return ERR_NOT_SUPPORTED;
	}

	if (offset > item->len) {
		LTRACEF("invalid offset %d\n", offset);
		return ERR_INVALID_ARGS;
	}

	const uint8_t *buf = msg_queue_get_buf(mq, item) + offset;
	size_t bytes_left = item->len - offset;

	if (msg->type == IPC_MSG_BUFFER_KERNEL) {
		if (msg->kern.num_handles) {
			LTRACEF("handles are not supported yet\n");
			return ERR_NOT_SUPPORTED;
		}
		return membuf_to_kern_iovec((const iovec_kern_t *)msg->kern.iov,
		                            msg->kern.num_iov,
		                            buf, bytes_left);
	} else if (msg->type == IPC_MSG_BUFFER_USER) {
		if (msg->user.num_handles) {
			LTRACEF("handles are not supported yet\n");
			return ERR_NOT_SUPPORTED;
		}
		return membuf_to_user_iovec(msg->user.iov, msg->user.num_iov,
		                            buf, bytes_left);
	} else {
		return ERR_INVALID_ARGS;
	}
}


/*
 *  Is called to look at the head of the filled messages list. It should be followed by
 *  calling msg_get_filled_locked call to actually move message to readable list.
 */
static int msg_peek_next_filled_locked(ipc_msg_queue_t *mq, ipc_msg_info_t *info)
{
	msg_item_t *item;

	item = list_peek_head_type(&mq->filled_list, msg_item_t, node);
	if (!item)
		return ERR_NO_MSG;

	info->len = item->len;
	info->id  = item->id;

	return NO_ERROR;
}


/*
 *  Is called to move top of the queue item to readable list.
 */
static void msg_get_filled_locked(ipc_msg_queue_t *mq)
{
	msg_item_t *item;

	item = list_peek_head_type(&mq->filled_list, msg_item_t, node);
	DEBUG_ASSERT(item);

	list_delete(&item->node);
	list_add_tail(&mq->read_list, &item->node);
	item->state = MSG_ITEM_STATE_READ;
}

static int msg_put_read_locked(ipc_chan_t *chan, uint32_t msg_id)
{
	DEBUG_ASSERT(chan);
	DEBUG_ASSERT(chan->msg_queue);

	ipc_msg_queue_t *mq = chan->msg_queue;
	msg_item_t *item = msg_queue_get_item(mq, msg_id);

	if (!item || item->state != MSG_ITEM_STATE_READ)
		return ERR_INVALID_ARGS;

	list_delete(&item->node);

	/* put it on the head since it was just taken off here */
	list_add_head(&mq->free_list, &item->node);
	item->state = MSG_ITEM_STATE_FREE;

	ipc_chan_t *peer = chan->peer;
	if (peer && (peer->aux_state & IPC_CHAN_AUX_STATE_SEND_BLOCKED)) {
		peer->aux_state &= ~IPC_CHAN_AUX_STATE_SEND_BLOCKED;
		peer->aux_state |=  IPC_CHAN_AUX_STATE_SEND_UNBLOCKED;
		handle_notify(&peer->handle);
	}

	return NO_ERROR;
}


long __SYSCALL sys_send_msg(uint32_t handle_id, user_addr_t user_msg)
{
	handle_t  *chandle;
	msg_desc_t tmp_msg;
	int ret;

	/* copy message descriptor from user space */
	tmp_msg.type = IPC_MSG_BUFFER_USER;
	ret = copy_from_user(&tmp_msg.user, user_msg, sizeof(ipc_msg_user_t));
	if (unlikely(ret != NO_ERROR))
		return (long) ret;

	/* grab handle */
	ret = uctx_handle_get(current_uctx(), handle_id, &chandle);
	if (unlikely(ret != NO_ERROR))
		return (long) ret;

	mutex_acquire(&ipc_lock);
	/* check if it is  avalid channel to call send_msg */
	ret = check_channel_connected_locked(chandle);
	if (likely(ret == NO_ERROR)) {
		ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);
		/* do write message to target channel  */
		ret = msg_write_locked(chan, &tmp_msg);
		if (ret >= 0) {
			/* and notify target */
			handle_notify(&chan->peer->handle);
		}
	}
	mutex_release(&ipc_lock);
	handle_decref(chandle);
	return (long) ret;
}

int ipc_send_msg(handle_t *chandle, ipc_msg_kern_t *msg)
{
	int ret;
	msg_desc_t tmp_msg;

	if (!msg)
		return ERR_INVALID_ARGS;

	tmp_msg.type = IPC_MSG_BUFFER_KERNEL;
	memcpy(&tmp_msg.kern, msg, sizeof(ipc_msg_kern_t));

	mutex_acquire(&ipc_lock);
	ret = check_channel_connected_locked(chandle);
	if (likely(ret == NO_ERROR)) {
		ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);
		ret = msg_write_locked(chan, &tmp_msg);
		if (ret >= 0) {
			handle_notify(&chan->peer->handle);
		}
	}
	mutex_release(&ipc_lock);
	return ret;
}

long __SYSCALL sys_get_msg(uint32_t handle_id, user_addr_t user_msg_info)
{
	handle_t *chandle;
	ipc_msg_info_t msg_info;
	int ret;

	/* grab handle */
	ret = uctx_handle_get(current_uctx(), handle_id, &chandle);
	if (ret != NO_ERROR)
		return (long) ret;

	mutex_acquire(&ipc_lock);
	/* check if channel handle is a valid one */
	ret = check_channel_locked(chandle);
	if (likely(ret == NO_ERROR)) {
		ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);
		/* peek next filled message */
		ret = msg_peek_next_filled_locked(chan->msg_queue, &msg_info);
		if (likely(ret == NO_ERROR)) {
			/* copy it to user space */
			ret = copy_to_user(user_msg_info,
					   &msg_info, sizeof(ipc_msg_info_t));
			if (likely(ret == NO_ERROR)) {
				/* and make it readable */
				msg_get_filled_locked(chan->msg_queue);
			}
		}
	}
	mutex_release(&ipc_lock);
	handle_decref(chandle);
	return (long) ret;
}

int ipc_get_msg(handle_t *chandle, ipc_msg_info_t *msg_info)
{
	int ret;

	mutex_acquire(&ipc_lock);
	/* check if channel handle */
	ret = check_channel_locked(chandle);
	if (likely(ret == NO_ERROR)) {
		ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);
		/* peek next filled message */
		ret  = msg_peek_next_filled_locked(chan->msg_queue, msg_info);
		if (likely(ret == NO_ERROR)) {
			/* and make it readable */
			msg_get_filled_locked(chan->msg_queue);
		}
	}
	mutex_release(&ipc_lock);
	return ret;
}


long __SYSCALL sys_put_msg(uint32_t handle_id, uint32_t msg_id)
{
	handle_t *chandle;

	/* grab handle */
	int ret = uctx_handle_get(current_uctx(), handle_id, &chandle);
	if (unlikely(ret != NO_ERROR))
		return (long) ret;

	/* and put it to rest */
	ret = ipc_put_msg(chandle, msg_id);
	handle_decref(chandle);

	return (long) ret;
}

int ipc_put_msg(handle_t *chandle, uint32_t msg_id)
{
	int ret;

	mutex_acquire(&ipc_lock);
	/* check is channel handle is a valid one */
	ret = check_channel_locked(chandle);
	if (likely(ret == NO_ERROR)) {
		ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);
		/* retire message */
		ret = msg_put_read_locked(chan, msg_id);
	}
	mutex_release(&ipc_lock);
	return ret;
}


long __SYSCALL sys_read_msg(uint32_t handle_id, uint32_t msg_id, uint32_t offset,
                            user_addr_t user_msg)
{
	handle_t  *chandle;
	msg_desc_t tmp_msg;
	int ret;

	/* get msg descriptor form user space */
	tmp_msg.type = IPC_MSG_BUFFER_USER;
	ret = copy_from_user(&tmp_msg.user, user_msg, sizeof(ipc_msg_user_t));
	if (unlikely(ret != NO_ERROR))
		return (long) ret;

	/* grab handle */
	ret = uctx_handle_get(current_uctx(), handle_id, &chandle);
	if (unlikely(ret != NO_ERROR))
		return (long) ret;

	mutex_acquire(&ipc_lock);
	/* check if channel handle is a valid one */
	ret = check_channel_locked (chandle);
	if (ret == NO_ERROR) {
		ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);
		/* read message content */
		ret = msg_read_locked(chan->msg_queue, msg_id,
		                      offset, &tmp_msg);
	}
	mutex_release(&ipc_lock);
	handle_decref(chandle);

	return (long) ret;
}

int ipc_read_msg(handle_t *chandle, uint32_t msg_id, uint32_t offset,
                 ipc_msg_kern_t *msg)
{
	int ret;
	msg_desc_t tmp_msg;

	if (!msg)
		return ERR_INVALID_ARGS;

	tmp_msg.type = IPC_MSG_BUFFER_KERNEL;
	memcpy(&tmp_msg.kern, msg, sizeof(ipc_msg_kern_t));

	mutex_acquire(&ipc_lock);
	ret = check_channel_locked (chandle);
	if (ret == NO_ERROR) {
		ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);
		ret = msg_read_locked(chan->msg_queue, msg_id,
		                      offset, &tmp_msg);
	}
	mutex_release(&ipc_lock);
	return ret;
}

#else /* WITH_TRUSTY_IPC */

long __SYSCALL sys_send_msg(uint32_t handle_id, user_addr_t user_msg)
{
	return (long) ERR_NOT_SUPPORTED;
}

long __SYSCALL sys_get_msg(uint32_t handle_id, user_addr_t user_msg_info)
{
	return (long) ERR_NOT_SUPPORTED;
}

long __SYSCALL sys_put_msg(uint32_t handle_id, uint32_t msg_id)
{
	return (long) ERR_NOT_SUPPORTED;
}

long __SYSCALL sys_read_msg(uint32_t handle_id, uint32_t msg_id, uint32_t offset,
                            user_addr_t user_msg)
{
	return (long) ERR_NOT_SUPPORTED;
}

#endif  /* WITH_TRUSTY_IPC */



