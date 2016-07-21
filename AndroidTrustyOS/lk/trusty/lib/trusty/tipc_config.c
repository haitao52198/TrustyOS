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

#include <err.h>
#include <assert.h>
#include <trace.h>

#include <lk/init.h>
#include <lib/trusty/tipc_dev.h>

/* Default TIPC device (/dev/trusty-ipc-dev0) */
DECLARE_TIPC_DEVICE_DESCR(_descr0, 0, 32, 32, "dev0");

/*
 *  Returns true if uuid is associated with NS client.
 */
bool is_ns_client(const uuid_t *uuid)
{
	if (uuid == &zero_uuid)
		return true;

	return false;
}

static void tipc_init(uint level)
{
	status_t res;

	res = create_tipc_device(&_descr0, sizeof(_descr0), &zero_uuid, NULL);
	if (res != NO_ERROR) {
		TRACEF("WARNING: failed (%d) to register tipc device\n", res);
	}
}

LK_INIT_HOOK_FLAGS(tipc_init, tipc_init,
                   LK_INIT_LEVEL_APPS-2, LK_INIT_FLAG_PRIMARY_CPU);

