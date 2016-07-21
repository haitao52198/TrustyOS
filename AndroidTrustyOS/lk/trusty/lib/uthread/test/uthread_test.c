/*
 * Copyright (c) 2013, Google Inc. All rights reserved
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

#include <debug.h>
#include <sys/types.h>
#include <uthread.h>

extern void umain(void);

void uthread_test(void)
{
	status_t err;
	vaddr_t entry = 0x8000;

	uthread_t *ut1 = uthread_create("test_ut1", 0x8000,
				DEFAULT_PRIORITY, 0x1000000, 4096, NULL);

	if (!ut1)
		return;

	/* Map the code section */
	err = uthread_map_contig(ut1, &entry, kvaddr_to_paddr(umain),
			(size_t)0x1000,
			UTM_R | UTM_W| UTM_X | UTM_FIXED,
			UT_MAP_ALIGN_DEFAULT);

	if (err) {
		dprintf(CRITICAL, "Error mapping code section %d\n", err);
		return;
	}


	/* Map in another time and remove it to test map/unmap */
	vaddr_t map_addr1, map_addr2;
	err = uthread_map_contig(ut1, &map_addr1, kvaddr_to_paddr(umain),
			(size_t)0x1000,
			UTM_R | UTM_W | UTM_X,
			UT_MAP_ALIGN_DEFAULT);

	if (err) {
		dprintf(CRITICAL, "Error mapping sample segment (1) %d\n",
				err);
		return;
	}

	dprintf(SPEW, "uthread_map_contig returned map_addr1 = 0x%lx\n", map_addr1);

	err = uthread_map_contig(ut1, &map_addr2, kvaddr_to_paddr(umain),
			(size_t)0x10000,
			UTM_R | UTM_W | UTM_X,
			UT_MAP_ALIGN_1MB);

	if (err) {
		dprintf(CRITICAL, "Error mapping sample segment (2) %d\n",
				err);
		return;
	}

	dprintf(SPEW, "uthread_map_contig returned map_addr2 = 0x%lx\n", map_addr2);

	/* Start unmapping sample segments */
	err = uthread_unmap(ut1, map_addr1, 0x1000);

	if(err) {
		dprintf(CRITICAL, "Error unmapping sample segment (1) %d\n",
				err);
		return;
	} else {
		dprintf(CRITICAL, "Successfully unmapped sample segment (1) \n");
	}

	err = uthread_unmap(ut1, map_addr2, 0x10000);

	if(err) {
		dprintf(CRITICAL, "Error unmapping sample segment (2) %d\n",
				err);
		return;
	} else {
		dprintf(CRITICAL, "Successfully unmapped sample segment (2)\n");
	}

	uthread_t *ut2 = uthread_create("test_ut2", 0x8000,
			DEFAULT_PRIORITY, 0x1000000, 4096, NULL);

	if(!ut2)
		return;

	err = uthread_map_contig(ut2, &entry, kvaddr_to_paddr(umain), 0x1000,
			UTM_R | UTM_W | UTM_X | UTM_FIXED,
			UT_MAP_ALIGN_DEFAULT);

	if (err) {
		dprintf(CRITICAL, "Error mapping code section %d\n", err);
		return;
	}

	/* start the uthread! */
	uthread_start(ut1);
	uthread_start(ut2);
}

