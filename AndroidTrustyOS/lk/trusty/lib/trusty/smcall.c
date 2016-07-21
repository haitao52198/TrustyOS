/*
 * Copyright (c) 2013-2015, Google, Inc. All rights reserved
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
#include <stddef.h>
#include <stdlib.h>
#include <sys/types.h>
#include <trace.h>
#include <lk/init.h>
#include <arch/mmu.h>
#include <lib/sm.h>
#include <lib/sm/smcall.h>

#include "trusty_virtio.h"

#define LOCAL_TRACE 0

/*
 * NS buffer helper function
 */
static status_t get_ns_mem_buf(struct smc32_args *args,
			       ns_addr_t *ppa, ns_size_t *psz, uint *pflags)
{
	DEBUG_ASSERT(ppa);
	DEBUG_ASSERT(psz);
	DEBUG_ASSERT(pflags);

	status_t rc = smc32_decode_mem_buf_info(args, ppa, psz, pflags);
	if (rc != NO_ERROR) {
		LTRACEF("Failed (%d) to decode mem buf info\n", rc);
		return rc;
	}

	/* We expect NORMAL CACHED or UNCHACHED RW EL1 memory */
	uint mem_type = *pflags & ARCH_MMU_FLAG_CACHE_MASK;

	if (mem_type != ARCH_MMU_FLAG_CACHED &&
	    mem_type != ARCH_MMU_FLAG_UNCACHED) {
		LTRACEF("Unexpected memory type: 0x%x\n", *pflags);
		return ERR_INVALID_ARGS;
	}

	if ((*pflags & ARCH_MMU_FLAG_PERM_RO) ||
	    (*pflags & ARCH_MMU_FLAG_PERM_USER)) {
		LTRACEF("Unexpected access attr: 0x%x\n", *pflags);
		return ERR_INVALID_ARGS;
	}

	return NO_ERROR;
}

/*
 *  Handle standard Trusted OS SMC call function
 */
static long trusty_sm_stdcall(smc32_args_t *args)
{
	long res;
	ns_size_t ns_sz;
	ns_paddr_t ns_pa;
	uint ns_mmu_flags;

	LTRACEF("Trusty SM service func %u args 0x%x 0x%x 0x%x\n",
		SMC_FUNCTION(args->smc_nr),
		args->params[0],
		args->params[1],
		args->params[2]);

	switch (args->smc_nr) {

	case SMC_SC_VIRTIO_GET_DESCR:
		res = get_ns_mem_buf(args, &ns_pa, &ns_sz, &ns_mmu_flags);
		if (res == NO_ERROR)
			res = virtio_get_description(ns_pa, ns_sz, ns_mmu_flags);
		break;

	case SMC_SC_VIRTIO_START:
		res = get_ns_mem_buf(args, &ns_pa, &ns_sz, &ns_mmu_flags);
		if (res == NO_ERROR)
			res = virtio_start(ns_pa, ns_sz, ns_mmu_flags);
		break;

	case SMC_SC_VIRTIO_STOP:
		res = get_ns_mem_buf(args, &ns_pa, &ns_sz, &ns_mmu_flags);
		if (res == NO_ERROR)
			res = virtio_stop(ns_pa, ns_sz, ns_mmu_flags);
		break;

	case SMC_SC_VDEV_RESET:
		res = virtio_device_reset(args->params[0]);
		break;

	case SMC_SC_VDEV_KICK_VQ:
		res = virtio_kick_vq(args->params[0], args->params[1]);
		break;

	default:
		LTRACEF("unknown func 0x%x\n", SMC_FUNCTION(args->smc_nr));
		res = ERR_NOT_SUPPORTED;
		break;
	}

	return res;
}

static smc32_entity_t trusty_sm_entity = {
	.stdcall_handler = trusty_sm_stdcall,
};

static void trusty_sm_init(uint level)
{
	int err;

	dprintf(INFO, "Initializing Trusted OS SMC handler\n");

	err = sm_register_entity(SMC_ENTITY_TRUSTED_OS, &trusty_sm_entity);
	if (err) {
		TRACEF("WARNING: Cannot register SMC entity! (%d)\n", err);
	}
}
LK_INIT_HOOK(trusty_smcall, trusty_sm_init, LK_INIT_LEVEL_APPS);

