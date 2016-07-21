/*
 * Copyright (c) 2013 Google Inc. All rights reserved
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
#ifndef __SM_H
#define __SM_H

#include <stdbool.h>
#include <stddef.h>
#include <sys/types.h>
#include <lib/sm/smcall.h>

typedef uint64_t ns_addr_t;
typedef uint32_t ns_size_t;

typedef struct ns_page_info {
	uint64_t attr;
} ns_page_info_t;

typedef struct smc32_args {
	uint32_t smc_nr;
	uint32_t params[SMC_NUM_PARAMS];
} smc32_args_t;

typedef long (*smc32_handler_t)(smc32_args_t *args);

typedef struct smc32_entity {
	smc32_handler_t fastcall_handler;
	smc32_handler_t stdcall_handler;
} smc32_entity_t;

/* Schedule Secure OS */
long sm_sched_secure(smc32_args_t *args);

/* Schedule Non-secure OS */
void sm_sched_nonsecure(long retval, smc32_args_t *args);

/* Handle an interrupt */
enum handler_return sm_handle_irq(void);
void sm_handle_fiq(void);

/* Version */
long smc_sm_api_version(smc32_args_t *args);

/* Interrupt controller irq/fiq support */
long smc_intc_get_next_irq(smc32_args_t *args);
long smc_intc_request_fiq(smc32_args_t *args);
status_t sm_intc_fiq_enter(void); /* return 0 to enter ns-fiq handler, return non-0 to return */
void sm_intc_fiq_exit(void);

/* Get the argument block passed in by the bootloader */
status_t sm_get_boot_args(void **boot_argsp, size_t *args_sizep);

/* Release bootloader arg block */
void sm_put_boot_args(void);

/* Register handler(s) for an entity */
status_t sm_register_entity(uint entity_nr, smc32_entity_t *entity);

/* Helper function to get NS memory buffer info out of smc32 call params */
status_t smc32_decode_mem_buf_info(struct smc32_args *args, ns_addr_t *ppa,
                                   ns_size_t *psz, uint *pmmu);

#endif /* __SM_H */

