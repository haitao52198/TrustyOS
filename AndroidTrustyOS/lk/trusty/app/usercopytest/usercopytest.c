/*
 * Copyright (c) 2015, Google Inc. All rights reserved
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

#include <assert.h>
#include <lk/init.h>
#include <stdio.h>
#include <string.h>
#include <uthread.h>

static void usercopy_test(uint level)
{
	int ret;
	char testbuf1[16];
	char testbuf2[16];

	memset(testbuf1, 0x11, sizeof(testbuf1));
	memset(testbuf2, 0x22, sizeof(testbuf2));
	ret = copy_to_user((user_addr_t)(uintptr_t)testbuf2, testbuf1, sizeof(testbuf1));
	ASSERT(ret == ERR_FAULT);
	ASSERT(testbuf1[0] == 0x11);
	ASSERT(testbuf2[0] == 0x22);

	memset(testbuf1, 0x11, sizeof(testbuf1));
	memset(testbuf2, 0x22, sizeof(testbuf2));
	ret = copy_from_user(testbuf1, (user_addr_t)(uintptr_t)testbuf2, sizeof(testbuf1) - 1);
	ASSERT(ret == ERR_FAULT);
	ASSERT(testbuf1[0] == 0x00);
	ASSERT(testbuf1[sizeof(testbuf1) - 2] == 0x00);
	ASSERT(testbuf1[sizeof(testbuf1) - 1] == 0x11);
	ASSERT(testbuf2[0] == 0x22);

	memset(testbuf1, 0x11, sizeof(testbuf1));
	memset(testbuf2, 0x22, sizeof(testbuf2));
	ret = strlcpy_from_user(testbuf1, (user_addr_t)(uintptr_t)testbuf2, sizeof(testbuf1) - 1);
	ASSERT(ret == ERR_FAULT);
	ASSERT(testbuf1[0] == 0x00);
	ASSERT(testbuf1[sizeof(testbuf1) - 2] == 0x00);
	ASSERT(testbuf1[sizeof(testbuf1) - 1] == 0x11);
	ASSERT(testbuf2[0] == 0x22);
	printf("PASSED - %s\n", __func__);
}

LK_INIT_HOOK(usercopy_test, usercopy_test, LK_INIT_LEVEL_APPS);
