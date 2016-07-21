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

#include <assert.h>
#include <err.h>
#include <list.h>
#include <stdlib.h>
#include <string.h>
#include <trace.h>
#include <uthread.h>
#include <platform.h>

#include <lk/init.h>
#include <kernel/mutex.h>
#include <kernel/event.h>

#include <lib/syscall.h>

#include <lib/trusty/uuid.h>

#if WITH_TRUSTY_IPC

#include <lib/trusty/ipc.h>
#include <lib/trusty/trusty_app.h>
#include <lib/trusty/uctx.h>
#include <lib/trusty/tipc_dev.h>

#include <reflist.h>

static struct list_node waiting_for_port_chan_list = LIST_INITIAL_VALUE(waiting_for_port_chan_list);

static struct list_node ipc_port_list = LIST_INITIAL_VALUE(ipc_port_list);

mutex_t ipc_lock = MUTEX_INITIAL_VALUE(ipc_lock);

static uint32_t port_poll(handle_t *handle);
static void port_shutdown(handle_t *handle);
static void port_handle_destroy(handle_t *handle);

static uint32_t chan_poll(handle_t *handle);
static void chan_finalize_event(handle_t *handle, uint32_t event);
static void chan_shutdown(handle_t *handle);
static void chan_handle_destroy(handle_t *handle);

static ipc_port_t *port_find_locked(const char *path);
static int port_attach_client(ipc_port_t *port, ipc_chan_t *client);
static void chan_shutdown_locked(ipc_chan_t *chan);
static void chan_add_ref(ipc_chan_t *conn, obj_ref_t *ref);
static void chan_del_ref(ipc_chan_t *conn, obj_ref_t *ref);

static struct handle_ops ipc_port_handle_ops = {
	.poll		= port_poll,
	.shutdown	= port_shutdown,
	.destroy	= port_handle_destroy,
};

static struct handle_ops ipc_chan_handle_ops = {
	.poll		= chan_poll,
	.finalize_event = chan_finalize_event,
	.shutdown	= chan_shutdown,
	.destroy	= chan_handle_destroy,
};

bool ipc_is_channel(handle_t *handle)
{
	return likely(handle->ops == &ipc_chan_handle_ops);
}

bool ipc_is_port(handle_t *handle)
{
	return likely(handle->ops == &ipc_port_handle_ops);
}

/*
 *  Called by user task to create a new port at the given path.
 *  The returned handle will be later installed into uctx.
 */
int ipc_port_create(const uuid_t *sid, const char *path,
                    uint num_recv_bufs, size_t recv_buf_size,
                    uint32_t flags,  handle_t **phandle_ptr)
{
	ipc_port_t *new_port;
	int ret = 0;

	LTRACEF("creating port (%s)\n", path);

	if (!sid) {
		/* server uuid is required */
		LTRACEF("server uuid is required\n");
		return ERR_INVALID_ARGS;
	}

	if (!num_recv_bufs || num_recv_bufs > IPC_CHAN_MAX_BUFS ||
	    !recv_buf_size || recv_buf_size > IPC_CHAN_MAX_BUF_SIZE) {
		LTRACEF("Invalid buffer sizes: %d x %zd\n",
		        num_recv_bufs, recv_buf_size);
		return ERR_INVALID_ARGS;
	}

	new_port = calloc(1, sizeof(ipc_port_t));
	if (!new_port) {
		LTRACEF("cannot allocate memory for port\n");
		return ERR_NO_MEMORY;
	}

	ret = strlcpy(new_port->path, path, sizeof(new_port->path));
	if (ret == 0) {
		LTRACEF("path is empty\n");
		ret = ERR_INVALID_ARGS;
		goto err_copy_path;
	}

	if ((uint) ret >= sizeof(new_port->path)) {
		LTRACEF("path is too long (%d)\n", ret);
		ret = ERR_TOO_BIG;
		goto err_copy_path;
	}

	new_port->uuid = sid;
	new_port->num_recv_bufs = num_recv_bufs;
	new_port->recv_buf_size = recv_buf_size;
	new_port->flags = flags;

	new_port->state = IPC_PORT_STATE_INVALID;
	list_initialize(&new_port->pending_list);

	handle_init(&new_port->handle, &ipc_port_handle_ops);

	LTRACEF("new port %p created (%s)\n", new_port, new_port->path);

	*phandle_ptr = &new_port->handle;

	return NO_ERROR;

err_copy_path:
	free(new_port);
	return ret;
}


/*
 * Shutting down port
 *
 * Called by controlling handle gets closed.
 */
static void port_shutdown(handle_t *phandle)
{
	ASSERT(phandle);
	ASSERT(ipc_is_port(phandle));

	mutex_acquire(&ipc_lock);

	ipc_port_t *port = containerof(phandle, ipc_port_t, handle);

	LTRACEF("shutting down port %p\n", port);

	/* change status to closing  */
	port->state = IPC_PORT_STATE_CLOSING;

	/* detach it from global list if it is in the list */
	if (list_in_list(&port->node)) {
		list_delete(&port->node);
		handle_decref(phandle);
	}

	/* tear down pending connections */
	ipc_chan_t *server, *temp;
	list_for_every_entry_safe(&port->pending_list, server, temp, ipc_chan_t, node) {
		/* pending server channel in not in user context table
		   so we can just call shutdown and delete it. Client
		   side will be deleted  by the other side
		 */
		chan_shutdown_locked(server);

		/* remove connection from the list */
		list_delete(&server->node);
		chan_del_ref(server, &server->node_ref); /* drop list ref */

		/* decrement usage count on port as pending connection
		   is gone
		 */
		handle_decref(phandle);
	}

	mutex_release(&ipc_lock);
}

/*
 * Destroy port controlled by handle
 *
 * Called when controlling handle refcount reaches 0.
 */
static void port_handle_destroy(handle_t *phandle)
{
	ASSERT(phandle);
	ASSERT(ipc_is_port(phandle));

	ipc_port_t *port = containerof(phandle, ipc_port_t, handle);

	/* pending list should be empty and
	   node should not be in the list
	 */
	DEBUG_ASSERT(list_is_empty(&port->pending_list));
	DEBUG_ASSERT(!list_in_list(&port->node));

	LTRACEF("destroying port %p ('%s')\n", port, port->path);

	free(port);
}

/*
 *   Make specified port publically available for operation.
 */
int ipc_port_publish(handle_t *phandle)
{
	int ret = NO_ERROR;

	DEBUG_ASSERT(phandle);
	DEBUG_ASSERT(ipc_is_port(phandle));

	mutex_acquire(&ipc_lock);

	ipc_port_t *port = containerof(phandle, ipc_port_t, handle);
	DEBUG_ASSERT(!list_in_list(&port->node));

	/* Check for duplicates */
	if (port_find_locked(port->path)) {
		LTRACEF("path already exists\n");
		ret = ERR_ALREADY_EXISTS;
	} else {
		port->state = IPC_PORT_STATE_LISTENING;
		list_add_tail(&ipc_port_list, &port->node);
		handle_incref(&port->handle); /* and inc usage count */

		/* go through pending connection list and pick those we can handle */
		ipc_chan_t *client, *temp;
		obj_ref_t tmp_client_ref = OBJ_REF_INITIAL_VALUE(tmp_client_ref);
		list_for_every_entry_safe(&waiting_for_port_chan_list, client, temp, ipc_chan_t, node) {

			if (strcmp(client->path, port->path))
				continue;

			free((void *)client->path);
			client->path = NULL;

			/* take it out of global pending list */
			chan_add_ref(client, &tmp_client_ref);   /* add local ref */
			list_delete(&client->node);
			chan_del_ref(client, &client->node_ref); /* drop list ref */

			/* try to attach port */
			int err = port_attach_client(port, client);
			if (err) {
				/* failed to attach port: close channel */
				LTRACEF("failed (%d) to attach_port\n", err);
				chan_shutdown_locked(client);
			}

			chan_del_ref(client, &tmp_client_ref);   /* drop local ref */
		}
	}
	mutex_release(&ipc_lock);

	return ret;
}


/*
 *  Called by user task to create new port.
 *
 *  On success - returns handle id (small integer) for the new port.
 *  On error   - returns negative error code.
 */
long __SYSCALL sys_port_create(user_addr_t path, uint num_recv_bufs,
                               size_t recv_buf_size, uint32_t flags)
{
	uthread_t *ut = uthread_get_current();
	trusty_app_t *tapp = ut->private_data;
	uctx_t *ctx = current_uctx();
	handle_t *port_handle = NULL;
	int ret;
	handle_id_t handle_id;
	char tmp_path[IPC_PORT_PATH_MAX];

	/* copy path from user space */
	ret = (int) strncpy_from_user(tmp_path, path, sizeof(tmp_path));
	if (ret < 0)
		return (long) ret;

	if ((uint)ret >= sizeof(tmp_path)) {
		/* string is too long */
		return ERR_INVALID_ARGS;
	}

	/* create new port */
	ret = ipc_port_create(&tapp->props.uuid, tmp_path,
		              (uint) num_recv_bufs, recv_buf_size,
		              flags, &port_handle);
	if (ret != NO_ERROR)
		goto err_port_create;

	/* install handle into user context */
	ret = uctx_handle_install(ctx, port_handle, &handle_id);
	if (ret != NO_ERROR)
		goto err_install;

	/* publish for normal operation */
	ret = ipc_port_publish(port_handle);
	if (ret != NO_ERROR)
		goto err_publish;

	return (long) handle_id;

err_publish:
	(void) uctx_handle_remove(ctx, handle_id, &port_handle);
err_install:
	handle_decref(port_handle);
err_port_create:
	return (long) ret;
}

/*
 *  Look up and port with given name (ipc_lock must be held)
 */
static ipc_port_t *port_find_locked(const char *path)
{
	ipc_port_t *port;

	list_for_every_entry(&ipc_port_list, port, ipc_port_t, node) {
		if (!strcmp(path, port->path))
			return port;
	}
	return NULL;
}

static uint32_t port_poll(handle_t *phandle)
{
	DEBUG_ASSERT(phandle);
	DEBUG_ASSERT(ipc_is_port(phandle));

	ipc_port_t *port = containerof(phandle, ipc_port_t, handle);
	uint32_t events = 0;

	mutex_acquire(&ipc_lock);
	if (port->state != IPC_PORT_STATE_LISTENING)
		events |= IPC_HANDLE_POLL_ERROR;
	else if (!list_is_empty(&port->pending_list))
		events |= IPC_HANDLE_POLL_READY;
	LTRACEF("%s in state %d events %x\n", port->path, port->state, events);
	mutex_release(&ipc_lock);

	return events;
}

/*
 *  Channel ref counting
 */
static inline void __chan_destroy_refobj(obj_t *ref)
{
	ipc_chan_t *chan = containerof(ref, ipc_chan_t, refobj);

	/* should not point to peer */
	ASSERT(chan->peer == NULL);

	/* should not be in a list  */
	ASSERT(!list_in_list(&chan->node));

	if (chan->path)
		free((void *)chan->path);

	if (chan->msg_queue) {
		ipc_msg_queue_destroy(chan->msg_queue);
		chan->msg_queue = NULL;
	}
	free(chan);
}

static inline void chan_add_ref(ipc_chan_t *chan, obj_ref_t *ref)
{
	obj_add_ref(&chan->refobj, ref);
}

static inline void chan_del_ref(ipc_chan_t *chan, obj_ref_t *ref)
{
	obj_del_ref(&chan->refobj, ref, __chan_destroy_refobj);
}

/*
 *   Initialize channel handle
 */
static inline handle_t *chan_handle_init(ipc_chan_t *chan)
{
	handle_init(&chan->handle, &ipc_chan_handle_ops);
	chan_add_ref(chan, &chan->handle_ref);
	return &chan->handle;
}

/*
 *  Allocate and initialize new channel.
 */
static ipc_chan_t *chan_alloc(uint32_t flags, const uuid_t *uuid,
			      obj_ref_t *ref)
{
	ipc_chan_t *chan;

	chan = calloc(1, sizeof(ipc_chan_t));
	if (!chan)
		return NULL;

	/* init ref count */
	obj_init(&chan->refobj, ref);

	/* init refs */
	obj_ref_init(&chan->node_ref);
	obj_ref_init(&chan->peer_ref);
	obj_ref_init(&chan->handle_ref);

	chan->uuid  = uuid;
	chan->state = IPC_CHAN_STATE_INVALID;
	chan->flags = flags;

	return chan;
}

static void _chan_shutdown_locked(ipc_chan_t *chan)
{
	switch (chan->state) {
	case IPC_CHAN_STATE_CONNECTED:
	case IPC_CHAN_STATE_CONNECTING:
		chan->state = IPC_CHAN_STATE_DISCONNECTING;
		handle_notify(&chan->handle);
		break;
	case IPC_CHAN_STATE_WAITING_FOR_PORT:
		ASSERT(list_in_list(&chan->node));
		list_delete(&chan->node);
		chan_del_ref(chan, &chan->node_ref);
		chan->state = IPC_CHAN_STATE_DISCONNECTING;
		break;
	case IPC_CHAN_STATE_ACCEPTING:
		chan->state = IPC_CHAN_STATE_DISCONNECTING;
		break;
	default:
		/* no op */
		break;
	}
}

static void chan_shutdown_locked(ipc_chan_t *chan)
{
	LTRACEF("chan %p: peer %p\n", chan, chan->peer);

	_chan_shutdown_locked(chan);
	if (chan->peer) {
		_chan_shutdown_locked(chan->peer);
		chan_del_ref(chan->peer, &chan->peer_ref);
		chan->peer = NULL;
	}
}

/*
 *  Called when caller closes handle.
 */
static void chan_shutdown(handle_t *chandle)
{
	DEBUG_ASSERT(chandle);
	DEBUG_ASSERT(ipc_is_channel(chandle));

	mutex_acquire(&ipc_lock);

	ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);

	chan_shutdown_locked(chan);

	mutex_release(&ipc_lock);
}

static void chan_handle_destroy(handle_t *chandle)
{
	DEBUG_ASSERT(chandle);
	DEBUG_ASSERT(ipc_is_channel(chandle));

	ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);
	mutex_acquire(&ipc_lock);
	chan_del_ref(chan, &chan->handle_ref);
	mutex_release(&ipc_lock);
}

/*
 *  Poll channel state
 */
static uint32_t chan_poll(handle_t *chandle)
{
	DEBUG_ASSERT(chandle);
	DEBUG_ASSERT(ipc_is_channel(chandle));

	/* TODO: finer locking? */
	mutex_acquire(&ipc_lock);

	ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);

	uint32_t events = 0;

	if (chan->state == IPC_CHAN_STATE_INVALID) {
		/* channel is in invalid state */
		events |= IPC_HANDLE_POLL_ERROR;
		goto done;
	}

	/*  peer is closing connection */
	if (chan->state == IPC_CHAN_STATE_DISCONNECTING) {
		events |= IPC_HANDLE_POLL_HUP;
	}

	/* server accepted our connection */
	if (chan->aux_state & IPC_CHAN_AUX_STATE_CONNECTED) {
		events |= IPC_HANDLE_POLL_READY;
	}

	/* have a pending message? */
	if (chan->msg_queue && !ipc_msg_queue_is_empty(chan->msg_queue)) {
		events |= IPC_HANDLE_POLL_MSG;
	}

	/* check if we were send blocked */
	if (chan->aux_state & IPC_CHAN_AUX_STATE_SEND_UNBLOCKED) {
		events |= IPC_HANDLE_POLL_SEND_UNBLOCKED;
	}

done:
	mutex_release(&ipc_lock);
	return events;
}

static void chan_finalize_event(handle_t *chandle, uint32_t event)
{
	DEBUG_ASSERT(chandle);
	DEBUG_ASSERT(ipc_is_channel(chandle));

	if (event & (IPC_HANDLE_POLL_SEND_UNBLOCKED | IPC_HANDLE_POLL_READY)) {
		mutex_acquire(&ipc_lock);
		ipc_chan_t *chan = containerof(chandle, ipc_chan_t, handle);
		if (event & IPC_HANDLE_POLL_SEND_UNBLOCKED)
			chan->aux_state &= ~IPC_CHAN_AUX_STATE_SEND_UNBLOCKED;
		if (event & IPC_HANDLE_POLL_READY)
			chan->aux_state &= ~IPC_CHAN_AUX_STATE_CONNECTED;
		mutex_release(&ipc_lock);
	}
}


/*
 *  Check if connection to specified port is allowed
 */
static int check_access(ipc_port_t *port, const uuid_t *uuid)
{
	if (!uuid)
		return ERR_ACCESS_DENIED;

	if (is_ns_client(uuid)) {
		/* check if this port allows connection from NS clients */
		if (port->flags & IPC_PORT_ALLOW_NS_CONNECT)
			return NO_ERROR;
	} else {
		/* check if this port allows connection from Trusted Apps */
		if (port->flags & IPC_PORT_ALLOW_TA_CONNECT)
			return NO_ERROR;
	}

	return ERR_ACCESS_DENIED;
}

static int port_attach_client(ipc_port_t *port, ipc_chan_t *client)
{
	int ret;
	ipc_chan_t *server;
	obj_ref_t   tmp_server_ref = OBJ_REF_INITIAL_VALUE(tmp_server_ref);

	if (port->state != IPC_PORT_STATE_LISTENING) {
		LTRACEF("port %s is not in listening state (%d)\n",
			 port->path, port->state);
		return ERR_NOT_READY;
	}

	/* check if we are allowed to connect */
	ret = check_access(port, client->uuid);
	if (ret != NO_ERROR) {
		LTRACEF("access denied: %d\n", ret);
		return ret;
	}

	server = chan_alloc(IPC_CHAN_FLAG_SERVER, port->uuid, &tmp_server_ref);
	if (!server) {
		LTRACEF("failed to alloc server: %d\n", ret);
		return ERR_NO_MEMORY;
	}

	/* allocate msg queues */
	ret = ipc_msg_queue_create(port->num_recv_bufs,
				   port->recv_buf_size,
				   &client->msg_queue);
	if (ret != NO_ERROR) {
		LTRACEF("failed to alloc mq: %d\n", ret);
		goto err_client_mq;
	}

	ret = ipc_msg_queue_create(port->num_recv_bufs,
				   port->recv_buf_size,
				   &server->msg_queue);
	if (ret != NO_ERROR) {
		LTRACEF("failed to alloc mq: %d\n", ret);
		goto err_server_mq;
	}

	/* move server to accepting state */
	server->state = IPC_CHAN_STATE_ACCEPTING;

	/* move client to connecting state  */
	client->state = IPC_CHAN_STATE_CONNECTING;

	/* setup peer refs */
	chan_add_ref(server, &client->peer_ref);
	client->peer = server;

	chan_add_ref(client, &server->peer_ref);
	server->peer = client;

	/* and add server channel to pending connection list */
	chan_add_ref(server, &server->node_ref);
	list_add_tail(&port->pending_list, &server->node);

	/* bump a ref to the port while there's a pending connection */
	handle_incref(&port->handle);

	/* Notify port that there is a pending connection */
	handle_notify(&port->handle);

	chan_del_ref(server, &tmp_server_ref);
	return NO_ERROR;

err_server_mq:
	ipc_msg_queue_destroy(client->msg_queue);
	client->msg_queue = NULL;
err_client_mq:
	chan_del_ref(server, &tmp_server_ref);
	return ERR_NO_MEMORY;
}

/*
 * Client requests a connection to a port. It can be called in context
 * of user task as well as vdev RX thread.
 */
int ipc_port_connect_async(const uuid_t *cid, const char *path, size_t max_path,
			   uint flags, handle_t **chandle_ptr)
{
	ipc_port_t *port;
	ipc_chan_t *client;
	obj_ref_t   tmp_client_ref = OBJ_REF_INITIAL_VALUE(tmp_client_ref);
	int ret;

	if (!cid) {
		/* client uuid is required */
		LTRACEF("client uuid is required\n");
		return ERR_INVALID_ARGS;
	}

	size_t len = strnlen(path, max_path);
	if (len == 0 || len >= max_path) {
		/* unterminated string */
		LTRACEF("invalid path specified\n");
		return ERR_INVALID_ARGS;
	}
	/* After this point path is zero terminated */

	/* allocate channel pair */
	client = chan_alloc(0, cid, &tmp_client_ref);
	if (!client) {
		LTRACEF("failed to alloc client\n");
		return ERR_NO_MEMORY;
	}

	LTRACEF("Connecting to '%s'\n", path);

	mutex_acquire(&ipc_lock);

	port = port_find_locked(path);
	if (port) {
		/* found  */
		ret = port_attach_client(port, client);
		if (ret)
			goto err_attach_client;
	} else {
		if (!(flags & IPC_CONNECT_WAIT_FOR_PORT)) {
			ret = ERR_NOT_FOUND;
			goto err_find_ports;
		}

		/* port not found, add connection to waiting_for_port_chan_list */
		client->path = strdup(path);
		if (!client->path) {
			ret = ERR_NO_MEMORY;
			goto err_alloc_path;
		}

		/* add it to waiting for port list */
		client->state = IPC_CHAN_STATE_WAITING_FOR_PORT;
		list_add_tail(&waiting_for_port_chan_list, &client->node);
		chan_add_ref(client, &client->node_ref);
	}

	LTRACEF("new connection: client %p: peer %p\n",
		client, client->peer);

	/* success */
	*chandle_ptr = chan_handle_init(client);
	ret = NO_ERROR;

err_alloc_path:
err_attach_client:
err_find_ports:
	chan_del_ref(client, &tmp_client_ref);
	mutex_release(&ipc_lock);
	return ret;
}

/* returns handle id for the new channel */

#ifndef DEFAULT_IPC_CONNECT_WARN_TIMEOUT
#define DEFAULT_IPC_CONNECT_WARN_TIMEOUT   INFINITE_TIME
#endif

long __SYSCALL sys_connect(user_addr_t path, uint flags)
{
	uthread_t *ut = uthread_get_current();
	trusty_app_t *tapp = ut->private_data;
	uctx_t *ctx = current_uctx();
	handle_t *chandle;
	char tmp_path[IPC_PORT_PATH_MAX];
	int ret;
	handle_id_t handle_id;

	if (flags & ~IPC_CONNECT_MASK) {
		/* unsupported flags specified */
		return ERR_INVALID_ARGS;
	}

	ret = (int) strncpy_from_user(tmp_path, path, sizeof(tmp_path));
	if (ret < 0)
		return (long) ret;

	if ((uint)ret >= sizeof(tmp_path))
		return (long) ERR_INVALID_ARGS;

	ret = ipc_port_connect_async(&tapp->props.uuid,
				     tmp_path, sizeof(tmp_path),
				     flags, &chandle);
	if (ret != NO_ERROR)
		return (long) ret;

	if (!(flags & IPC_CONNECT_ASYNC)) {
		uint32_t event;
		lk_time_t timeout_msecs = DEFAULT_IPC_CONNECT_WARN_TIMEOUT;

		ret = handle_wait(chandle, &event, timeout_msecs);
		if (ret == ERR_TIMED_OUT) {
			TRACEF("Timedout connecting to %s\n", tmp_path);
			ret = handle_wait(chandle, &event, INFINITE_TIME);
		}

		if (ret < 0) {
			/* timeout or other error */
			handle_close(chandle);
			return ret;
		}

		if ((event & IPC_HANDLE_POLL_HUP) &&
		    !(event & IPC_HANDLE_POLL_MSG)) {
			/* hangup and no pending messages */
			handle_close(chandle);
			return ERR_CHANNEL_CLOSED;
		}

		if (!(event & IPC_HANDLE_POLL_READY)) {
			/* not connected */
			TRACEF("Unexpected channel state: event = 0x%x\n", event);
			handle_close(chandle);
			return ERR_NOT_READY;
		}
	}

	ret = uctx_handle_install(ctx, chandle, &handle_id);
	if (ret != NO_ERROR) {
		/* Failed to install handle into user context */
		handle_close(chandle);
		return (long) ret;
	}

	return (long) handle_id;
}

/*
 *  Called by user task to accept incomming connection
 */
int ipc_port_accept(handle_t *phandle, handle_t **chandle_ptr,
                    const uuid_t **uuid_ptr)
{
	ipc_port_t *port;
	ipc_chan_t *server = NULL;
	ipc_chan_t *client = NULL;
	obj_ref_t tmp_server_ref = OBJ_REF_INITIAL_VALUE(tmp_server_ref);
	int ret = NO_ERROR;

	DEBUG_ASSERT(chandle_ptr);
	DEBUG_ASSERT(uuid_ptr);

	if (!phandle || !ipc_is_port(phandle)) {
		LTRACEF("invalid port handle %p\n", phandle);
		return ERR_INVALID_ARGS;
	}

	port = containerof(phandle, ipc_port_t, handle);

	mutex_acquire(&ipc_lock);

	if (port->state != IPC_PORT_STATE_LISTENING) {
		/* Not in listening state: caller should close port.
		 * is it really possible to get here?
		 */
		ret = ERR_CHANNEL_CLOSED;
		goto err_bad_port_state;
	}

	/* get next pending connection */
	server = list_remove_head_type(&port->pending_list, ipc_chan_t, node);
	if (!server) {
		/* TODO: should we block waiting for a new connection if one
		 * is not pending? if so, need an optional argument maybe.
		 */
		ret = ERR_NO_MSG;
		goto err_no_connections;
	}

	/* it must be a server side channel */
	DEBUG_ASSERT(server->flags & IPC_CHAN_FLAG_SERVER);

	chan_add_ref(server, &tmp_server_ref);  /* add local ref */
	chan_del_ref(server, &server->node_ref);  /* drop list ref */

	/* drop the ref to port we took in connect() */
	handle_decref(&port->handle);

	client = server->peer;

	/* there must be a client, it must be in CONNECTING state and
	   server must be in ACCEPTING state */
	if (!client ||
	    server->state != IPC_CHAN_STATE_ACCEPTING ||
	    client->state != IPC_CHAN_STATE_CONNECTING) {
		LTRACEF("Drop connection: client %p (0x%x) to server %p (0x%x):\n",
			client, client ? client->state : 0xDEADBEEF,
			server, server->state);
		chan_shutdown_locked(server);
		ret = ERR_CHANNEL_CLOSED;
		goto err_bad_chan_state;
	}

	/* move both client and server into connected state */
	server->state = IPC_CHAN_STATE_CONNECTED;
	client->state = IPC_CHAN_STATE_CONNECTED;
	client->aux_state |= IPC_CHAN_AUX_STATE_CONNECTED;

	/* init server channel handle and return it to caller */
	*chandle_ptr = chan_handle_init(server);
	*uuid_ptr = client->uuid;

	/* notify client */
	handle_notify(&client->handle);

	ret = NO_ERROR;

err_bad_chan_state:
	chan_del_ref(server, &tmp_server_ref);
err_no_connections:
err_bad_port_state:
	mutex_release(&ipc_lock);
	return ret;
}

long __SYSCALL sys_accept(uint32_t handle_id, user_addr_t user_uuid)
{
	uctx_t *ctx = current_uctx();
	handle_t *phandle = NULL;
	handle_t *chandle = NULL;
	int ret;
	handle_id_t new_id;
	const uuid_t *peer_uuid_ptr;

	ret = uctx_handle_get(ctx, handle_id, &phandle);
	if (ret != NO_ERROR)
		return (long) ret;

	ret = ipc_port_accept(phandle, &chandle, &peer_uuid_ptr);
	if (ret != NO_ERROR)
		goto err_accept;

	ret = uctx_handle_install(ctx, chandle, &new_id);
	if (ret != NO_ERROR)
		goto err_install;

	/* copy peer uuid into userspace */
	ret = copy_to_user(user_uuid, peer_uuid_ptr, sizeof(uuid_t));
	if (ret != NO_ERROR)
		goto err_uuid_copy;

	handle_decref(phandle);
	return (long) new_id;

err_uuid_copy:
	uctx_handle_remove(ctx, new_id, &chandle);
err_install:
	handle_close(chandle);
err_accept:
	handle_decref(phandle);
	return (long) ret;
}

#else /* WITH_TRUSTY_IPC */

long __SYSCALL sys_port_create(user_addr_t path, uint num_recv_bufs,
                               size_t recv_buf_size, uint32_t flags)
{
	return (long) ERR_NOT_SUPPORTED;
}

long __SYSCALL sys_connect(user_addr_t path, uint flags)
{
	return (long) ERR_NOT_SUPPORTED;
}

long __SYSCALL sys_accept(uint32_t handle_id, uuid_t *peer_uuid)
{
	return (long) ERR_NOT_SUPPORTED;
}

#endif /* WITH_TRUSTY_IPC */

