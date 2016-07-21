/*
 * Copyright (c) 2013, Google, Inc. All rights reserved
 * Copyright (c) 2012-2013, NVIDIA CORPORATION. All rights reserved
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
#ifndef __LIB_TRUSTY_APP_MANIFEST_H
#define __LIB_TRUSTY_APP_MANIFEST_H

#include <sys/types.h>

#include "trusty_uuid.h"

/*
 * Layout of .trusty_app.manifest section in the trusted application is the
 * required UUID followed by an abitrary number of configuration options.
 */
typedef struct trusty_app_manifest {
	uuid_t uuid;
	uint32_t config_options[];
} trusty_app_manifest_t;

enum {
	TRUSTY_APP_CONFIG_KEY_MIN_STACK_SIZE	= 1,
	TRUSTY_APP_CONFIG_KEY_MIN_HEAP_SIZE	= 2,
	TRUSTY_APP_CONFIG_KEY_MAP_MEM		= 3,
};

#define TRUSTY_APP_CONFIG_MIN_STACK_SIZE(sz) \
	TRUSTY_APP_CONFIG_KEY_MIN_STACK_SIZE, sz

#define TRUSTY_APP_CONFIG_MIN_HEAP_SIZE(sz) \
	TRUSTY_APP_CONFIG_KEY_MIN_HEAP_SIZE, sz

#define TRUSTY_APP_CONFIG_MAP_MEM(id,off,sz) \
	TRUSTY_APP_CONFIG_KEY_MAP_MEM, id, off, sz

/* manifest section attributes */
#define TRUSTY_APP_MANIFEST_ATTRS \
	__attribute((aligned(4))) __attribute((section(".trusty_app.manifest")))

#endif

