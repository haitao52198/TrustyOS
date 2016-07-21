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
#include <debug.h>
#include <err.h>
#include <list.h> // for containerof
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <trace.h>

#include <kernel/event.h>
#include <kernel/wait.h>

#if WITH_TRUSTY_IPC

#include <lib/syscall.h>
#include <lib/trusty/ipc.h>

void handle_init(handle_t *handle, struct handle_ops *ops)
{
	DEBUG_ASSERT(handle);
	DEBUG_ASSERT(ops);
	DEBUG_ASSERT(ops->destroy);

	refcount_init(&handle->refcnt);
	handle->ops = ops;
	handle->wait_event = NULL;
	mutex_init(&handle->wait_event_lock);
	handle->cookie = NULL;
	list_clear_node(&handle->hlist_node);
}

static void __handle_destroy_ref(refcount_t *ref)
{
	DEBUG_ASSERT(ref);

	handle_t *handle = containerof(ref, handle_t, refcnt);
	handle->ops->destroy(handle);
}

void handle_incref(handle_t *handle)
{
	DEBUG_ASSERT(handle);
	refcount_inc(&handle->refcnt);
}

void handle_decref(handle_t *handle)
{
	DEBUG_ASSERT(handle);
	refcount_dec(&handle->refcnt, __handle_destroy_ref);
}

void handle_close(handle_t *handle)
{
	DEBUG_ASSERT(handle);
	if (handle->ops->shutdown)
		handle->ops->shutdown(handle);
	handle_decref(handle);
}

static int __do_wait(event_t *ev, lk_time_t timeout)
{
	int ret;

	LTRACEF("waiting\n");
	ret = event_wait_timeout(ev, timeout);
	LTRACEF("waited\n");
	return ret;
}

static int _prepare_wait_handle(event_t *ev, handle_t *handle)
{
	int ret = 0;

	mutex_acquire(&handle->wait_event_lock);
	if (unlikely(handle->wait_event)) {
		LTRACEF("someone is already waiting on handle %p?!\n",
			handle);
		ret = ERR_ALREADY_STARTED;
	} else {
		handle->wait_event = ev;
	}
	mutex_release(&handle->wait_event_lock);
	return ret;
}

static void _finish_wait_handle(handle_t *handle)
{
	/* clear out our event ptr */
	mutex_acquire(&handle->wait_event_lock);
	handle->wait_event = NULL;
	mutex_release(&handle->wait_event_lock);
}

int handle_wait(handle_t *handle, uint32_t *handle_event, lk_time_t timeout)
{
	uint32_t event;
	event_t ev;
	int ret = 0;

	if (!handle || !handle_event)
		return ERR_INVALID_ARGS;

	event_init(&ev, false, EVENT_FLAG_AUTOUNSIGNAL);

	ret = _prepare_wait_handle(&ev, handle);
	if (ret) {
		LTRACEF("someone is already waiting on handle %p\n", handle);
		goto err_prepare_wait;
	}

	while (true) {
		event = handle->ops->poll(handle);
		if (event)
			break;
		ret = __do_wait(&ev, timeout);
		if (ret < 0)
			goto finish_wait;
	}

	if (handle->ops->finalize_event)
		handle->ops->finalize_event(handle, event);
	*handle_event = event;
	ret = NO_ERROR;

finish_wait:
	_finish_wait_handle(handle);
err_prepare_wait:
	event_destroy(&ev);
	return ret;
}

void handle_notify(handle_t *handle)
{
	DEBUG_ASSERT(handle);

	/* TODO: need to keep track of the number of events posted? */

	mutex_acquire(&handle->wait_event_lock);
	if (handle->wait_event) {
		LTRACEF("notifying handle %p wait_event %p\n",
			handle, handle->wait_event);
		event_signal(handle->wait_event, true);
	}
	mutex_release(&handle->wait_event_lock);
}

void handle_list_init(handle_list_t *hlist)
{
	DEBUG_ASSERT(hlist);

	*hlist = (handle_list_t)HANDLE_LIST_INITIAL_VALUE(*hlist);
}

void handle_list_add(handle_list_t *hlist, handle_t *handle)
{
	DEBUG_ASSERT(hlist);
	DEBUG_ASSERT(handle);
	DEBUG_ASSERT(!list_in_list(&handle->hlist_node));

	handle_incref(handle);
	mutex_acquire(&hlist->lock);
	list_add_tail(&hlist->handles, &handle->hlist_node);
	if (hlist->wait_event) {
		/* somebody is waiting on list */
		_prepare_wait_handle(hlist->wait_event, handle);

		/* call poll to check if it is already signaled */
		uint32_t event = handle->ops->poll(handle);
		if (event) {
			handle_notify(handle);
		}
	}
	mutex_release(&hlist->lock);
}

static void _handle_list_del_locked(handle_list_t *hlist, handle_t *handle)
{
	DEBUG_ASSERT(hlist);
	DEBUG_ASSERT(handle);
	DEBUG_ASSERT(list_in_list(&handle->hlist_node));

	/* remove item from list */
	list_delete(&handle->hlist_node);

	/* check if somebody is waiting on this handle list */
	if (hlist->wait_event) {
		/* finish waiting */
		_finish_wait_handle(handle);
		if (list_is_empty(&hlist->handles)) {
			/* wakeup waiter if list is now empty */
			event_signal(hlist->wait_event, true);
		}
	}
	handle_decref(handle);
}

void handle_list_del(handle_list_t *hlist, handle_t *handle)
{
	DEBUG_ASSERT(hlist);
	DEBUG_ASSERT(handle);

	mutex_acquire(&hlist->lock);
	_handle_list_del_locked(hlist, handle);
	mutex_release(&hlist->lock);
}

void handle_list_delete_all(handle_list_t *hlist)
{
	DEBUG_ASSERT(hlist);

	mutex_acquire(&hlist->lock);
	while (!list_is_empty(&hlist->handles)) {
		handle_t *handle;

		handle = list_peek_head_type(&hlist->handles, handle_t,
					     hlist_node);
		_handle_list_del_locked(hlist, handle);
	}
	mutex_release(&hlist->lock);
}

/*
 *  Iterate handle list and call finish_wait for each item until the last one
 *  (inclusive) specified by corresponding function parameter. If the last item
 *  is not specified, iterate the whole list.
 */
static void _hlist_finish_wait_locked(handle_list_t *hlist, handle_t *last)
{
	handle_t *handle;
	list_for_every_entry(&hlist->handles, handle, handle_t, hlist_node) {
		_finish_wait_handle(handle);
		if (handle == last)
			break;
	}
}

/*
 *  Iterate handle list and call prepare wait (if required) and poll for each
 *  handle until the ready one is found and return it to caller.
 *  Undo prepare op if ready handle is found or en error occured.
 */
static int _hlist_do_poll_locked(handle_list_t *hlist, handle_t **handle_ptr,
				 uint32_t *event_ptr, bool prepare)
{
	int ret;

	DEBUG_ASSERT(hlist->wait_event);

	if (list_is_empty(&hlist->handles))
		return ERR_NOT_FOUND;  /* no handles in the list */

	handle_t *next;
	handle_t *last_prep = NULL;
	list_for_every_entry(&hlist->handles, next, handle_t, hlist_node) {
		if (prepare) {
			ret = _prepare_wait_handle(hlist->wait_event, next);
			if (ret)
				break;
			last_prep = next;
		}

		uint32_t event = next->ops->poll(next);
		if (event) {
			*event_ptr = event;
			*handle_ptr = next;
			ret = 1;
			break;
		}
	}

	if (ret && prepare && last_prep) {
		/* need to undo prepare */
		_hlist_finish_wait_locked(hlist, last_prep);
	}
	return ret;
}

/* fills in the handle that has a pending event. The reference taken by the list
 * is not dropped until the caller has had a chance to process the handle.
 */
int handle_list_wait(handle_list_t *hlist, handle_t **handle_ptr,
                     uint32_t *event_ptr, lk_time_t timeout)
{
	int ret;
	event_t ev;

	DEBUG_ASSERT(hlist);
	DEBUG_ASSERT(handle_ptr);
	DEBUG_ASSERT(event_ptr);

	event_init(&ev, false, EVENT_FLAG_AUTOUNSIGNAL);

	*event_ptr = 0;
	*handle_ptr = 0;

	mutex_acquire(&hlist->lock);

	DEBUG_ASSERT(hlist->wait_event == NULL);

	hlist->wait_event = &ev;
	ret = _hlist_do_poll_locked(hlist, handle_ptr, event_ptr, true);
	if (ret < 0)
		goto err_do_poll;

	if (ret == 0) {
		/* no handles ready */
		do {
			mutex_release(&hlist->lock);
			ret = __do_wait(&ev, timeout);
			mutex_acquire(&hlist->lock);

			if (ret < 0)
				break;

			/* poll again */
			ret = _hlist_do_poll_locked(hlist, handle_ptr,
						    event_ptr, false);
		} while (!ret);

		_hlist_finish_wait_locked(hlist, NULL);
	}

	if (ret == 1) {
		handle_t *handle = *handle_ptr;

		if (handle->ops->finalize_event)
			handle->ops->finalize_event(handle, *event_ptr);

		handle_incref(handle);

		/* move list head after item we just found */
		list_delete(&hlist->handles);
		list_add_head(&handle->hlist_node, &hlist->handles);

		ret = NO_ERROR;
	}

err_do_poll:
	hlist->wait_event = NULL;
	mutex_release(&hlist->lock);
	event_destroy(&ev);
	return ret;
}

#endif /* WITH_TRUSTY_IPC */

