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
#include <bits.h>
#include <err.h>
#include <list.h> // for containerof
#include <platform.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <trace.h>
#include <uthread.h>

#include <kernel/event.h>
#include <kernel/thread.h>
#include <kernel/wait.h>

#include <lib/syscall.h>

#if WITH_TRUSTY_IPC

#include <lk/init.h>
#include <lib/trusty/handle.h>
#include <lib/trusty/trusty_app.h>
#include <lib/trusty/uctx.h>

/* must be a multiple of sizeof(unsigned long) */
#define IPC_MAX_HANDLES		64

struct uctx {
	unsigned long		inuse[BITMAP_NUM_WORDS(IPC_MAX_HANDLES)];
	handle_t		*handles[IPC_MAX_HANDLES];

	void			*priv;
	handle_list_t		handle_list;
};

static status_t _uctx_startup(trusty_app_t *app);

static uint _uctx_slot_id;
static struct trusty_app_notifier _uctx_notifier = {
	.startup = _uctx_startup,
};

static status_t _uctx_startup(trusty_app_t *app)
{
	uctx_t *uctx;

	int err = uctx_create(app, &uctx);
	if (err)
		return err;

	trusty_als_set(app, _uctx_slot_id, uctx);
	return NO_ERROR;
}

static void uctx_init(uint level)
{
	int res;

	/* allocate als slot */
	res = trusty_als_alloc_slot();
	if (res < 0)
		panic("failed (%d) to alloc als slot\n", res);
	_uctx_slot_id = res;

	/* register notifier */
	res = trusty_register_app_notifier(&_uctx_notifier);
	if (res < 0)
		panic("failed (%d) to register uctx notifier\n", res);
}

LK_INIT_HOOK(uctx, uctx_init, LK_INIT_LEVEL_APPS - 2);

/*
 *  Get uctx context of the current app
 */
uctx_t *current_uctx(void)
{
	uthread_t *ut = uthread_get_current();
	trusty_app_t *tapp = ut->private_data;
	return trusty_als_get(tapp, _uctx_slot_id);
}

/*
 *  Check if specified handle_id does represent a valid handle
 *  for specified user context.
 */
static int _check_handle_id(uctx_t *ctx, handle_id_t handle_id)
{
	DEBUG_ASSERT(ctx);

	if (unlikely(handle_id >= IPC_MAX_HANDLES)) {
		LTRACEF("%d is invalid handle id\n", handle_id);
		return ERR_BAD_HANDLE;
	}

	if (!bitmap_test(ctx->inuse, handle_id)) {
		LTRACEF("%d is unused handle id\n", handle_id);
		return ERR_NOT_FOUND;
	}

	/* there should be a handle there */
	ASSERT(ctx->handles[handle_id]);

	return NO_ERROR;
}

/*
 *  Return handle id for specified handle
 */
static handle_id_t _handle_to_id_locked(uctx_t *ctx, handle_t *handle)
{
	DEBUG_ASSERT(ctx);

	for (int i = 0; i < IPC_MAX_HANDLES; i++) {
		if (ctx->handles[i] == handle) {
			return i;
		}
	}
	return INVALID_HANDLE_ID;
}


/*
 *  Allocate and initialize user context - the structure that is used
 *  to keep track handles on behalf of user space app. Exactly one user
 *  context is created for each trusty app during it's nitialization.
 */
int uctx_create(void *priv, uctx_t **ctx)
{
	uctx_t *new_ctx;

	DEBUG_ASSERT(ctx);

	new_ctx = calloc(1, sizeof(uctx_t));
	if (!new_ctx) {
		LTRACEF("Out of memory\n");
		return ERR_NO_MEMORY;
	}

	new_ctx->priv = priv;

	handle_list_init(&new_ctx->handle_list);

	*ctx = new_ctx;

	return NO_ERROR;
}

/*
 *   Destroy user context previously created by uctx_create.
 */
void uctx_destroy(uctx_t *ctx)
{
	int i;
	DEBUG_ASSERT(ctx);

	for (i = 0; i < IPC_MAX_HANDLES; i++) {
		if (ctx->handles[i]) {
			TRACEF("destroying a non-empty uctx!!!\n");
			handle_list_del(&ctx->handle_list, ctx->handles[i]);
			handle_close(ctx->handles[i]);
			ctx->handles[i] = NULL;
		}
	}
	free(ctx);
}

/*
 *  Returns private data associated with user context. (Currently unused)
 */
void *uctx_get_priv(uctx_t *ctx)
{
	ASSERT(ctx);
	return ctx->priv;
}

/*
 * The caller transfers an ownership (and thus its ref) of the handle
 * to this function, which is handed off to the handle table of the user
 * process.
 */
int uctx_handle_install(uctx_t *ctx, handle_t *handle, handle_id_t *id)
{
	int new_id;

	DEBUG_ASSERT(ctx);
	DEBUG_ASSERT(handle);
	DEBUG_ASSERT(id);

	new_id = bitmap_ffz(ctx->inuse, IPC_MAX_HANDLES);
	if (new_id < 0)
		return ERR_NO_RESOURCES;

	ASSERT(ctx->handles[new_id] == NULL);

	bitmap_set(ctx->inuse, new_id);
	ctx->handles[new_id] = handle;
	handle_list_add(&ctx->handle_list, handle);
	*id = (handle_id_t) new_id;

	return NO_ERROR;
}

/*
 *   Retrieve handle from specified user context specified by
 *   given handle_id. Increment ref count for returned handle.
 */
int uctx_handle_get(uctx_t *ctx, handle_id_t handle_id, handle_t **handle_ptr)
{
	DEBUG_ASSERT(ctx);
	DEBUG_ASSERT(handle_ptr);

	int ret = _check_handle_id (ctx, handle_id);
	if (ret == NO_ERROR) {
		handle_t *handle = ctx->handles[handle_id];
		/* take a reference on the handle we looked up */
		handle_incref(handle);
		*handle_ptr = handle;
	}

	return ret;
}

/*
 *  Detach handle specified by handle ID from given user context and
 *  return it to caller.
 */
int uctx_handle_remove(uctx_t *ctx, handle_id_t handle_id, handle_t **handle_ptr)
{
	DEBUG_ASSERT(ctx);
	DEBUG_ASSERT(handle_ptr);

	int ret = _check_handle_id(ctx, handle_id);
	if (ret == NO_ERROR) {
		handle_t *handle = ctx->handles[handle_id];
		bitmap_clear(ctx->inuse, handle_id);
		ctx->handles[handle_id] = NULL;
		handle_list_del(&ctx->handle_list, handle);
		*handle_ptr = handle;
	}

	return ret;
}


/******************************************************************************/

/* definition shared with userspace */
typedef struct uevent {
	uint32_t		handle;
	uint32_t		event;
	user_addr_t		cookie;
} uevent_t;

/*
 *   wait on single handle specified by handle id
 */
long __SYSCALL sys_wait(uint32_t handle_id, user_addr_t user_event,
                        unsigned long timeout_msecs)
{
	uctx_t *ctx = current_uctx();
	handle_t *handle;
	uevent_t tmp_event;
	int ret;

	LTRACEF("[%p][%d]: %ld msec\n", uthread_get_current(),
	                                handle_id, timeout_msecs);

	ret = uctx_handle_get(ctx, handle_id, &handle);
	if (ret != NO_ERROR)
		return ret;

	DEBUG_ASSERT(handle);

	ret = handle_wait(handle, &tmp_event.event,
		          timeout_msecs);
	if (ret < 0) {
		/* an error or no events (timeout) */
		goto out;
	}

	/* got an event */
	tmp_event.handle = handle_id;
	tmp_event.cookie = (user_addr_t)(uintptr_t)handle_get_cookie(handle);

	status_t status = copy_to_user(user_event, &tmp_event, sizeof(tmp_event));
	if (status) {
		/* failed to copy, propogate error to caller */
		ret = (long) status;
	}

out:
	/* drop handle_ref grabed by uctx_handle_get */
	handle_decref(handle);

	LTRACEF("[%p][%d]: ret = %d\n", uthread_get_current(),
	                                handle_id, ret);
	return ret;
}

/*
 *   Wait on any handle existing in user context.
 */
long __SYSCALL sys_wait_any(user_addr_t user_event, unsigned long timeout_msecs)
{
	uctx_t *ctx = current_uctx();
	handle_t *handle;
	uevent_t tmp_event;
	int ret;

	LTRACEF("[%p]: %ld msec\n", uthread_get_current(),
	                            timeout_msecs);

	/*
	 * Get a handle that has a pending event. The returned handle has
	 * extra ref taken.
	 */
	ret = handle_list_wait(&ctx->handle_list, &handle, &tmp_event.event,
			       timeout_msecs);
	if (ret < 0) {
		/* an error or no events (timeout) */
		goto out;
	}

	DEBUG_ASSERT(handle); /* there should be a handle */

	tmp_event.handle = _handle_to_id_locked(ctx, handle);
	tmp_event.cookie = (user_addr_t)(uintptr_t)handle_get_cookie(handle);

	/* drop the reference that was taken by wait_any */
	handle_decref(handle);

	/* there should be a handle id */
	DEBUG_ASSERT(tmp_event.handle < IPC_MAX_HANDLES);

	status_t status = copy_to_user(user_event, &tmp_event, sizeof(tmp_event));
	if (status) {
		/* failed to copy, propogate error to caller */
		ret = status;
	}
out:
	LTRACEF("[%p][%d]: ret = %d\n", uthread_get_current(),
	                                tmp_event.handle, ret);
	return ret;
}

long __SYSCALL sys_close(uint32_t handle_id)
{
	handle_t *handle;

	LTRACEF("[%p][%d]\n", uthread_get_current(),
	                      handle_id);

	int ret = uctx_handle_remove(current_uctx(), handle_id, &handle);
	if (ret != NO_ERROR)
		return ret;

	handle_close(handle);
	return NO_ERROR;
}

long __SYSCALL sys_set_cookie(uint32_t handle_id, user_addr_t cookie)
{
	handle_t *handle;

	LTRACEF("[%p][%d]: cookie = 0x%08x\n", uthread_get_current(),
	                              handle_id, (uint) cookie);

	int ret = uctx_handle_get(current_uctx(), handle_id, &handle);
	if (ret != NO_ERROR)
		return ret;

	handle_set_cookie(handle, (void *)(uintptr_t)cookie);

	handle_decref(handle);
	return NO_ERROR;
}

#else  /* WITH_TRUSTY_IPC */

long __SYSCALL sys_wait(uint32_t handle_id, user_addr_t user_event,
                        unsigned long timeout_msecs)
{
	return (long) ERR_NOT_SUPPORTED;
}

long __SYSCALL sys_wait_any(user_addr_t user_event, unsigned long timeout_msecs)
{
	return (long) ERR_NOT_SUPPORTED;
}


long __SYSCALL sys_close(uint32_t handle_id)
{
	return (long) ERR_NOT_SUPPORTED;
}

long __SYSCALL sys_set_cookie(uint32_t handle_id, user_addr_t cookie)
{
	return (long) ERR_NOT_SUPPORTED;
}

#endif /* WITH_TRUSTY_IPC */




