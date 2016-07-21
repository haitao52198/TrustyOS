/*
 * Copyright (c) 2015 Google Inc. All rights reserved
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

#ifndef __MM_H
#define __MM_H

/*
 * Flags for mmap syscall
 */

/*
 * Maps memory region specified by IO region id (handle) to user address space.
 * IO Mapping has fixed attributes (non-secure, strongly ordered,
 * physically contiguous, read-writable and 4k aligned) for now.
 */
#define MMAP_FLAG_IO_HANDLE		(0x1 << 0)

/**
 * struct dma_pmem - a contiguous physical memory block
 * @paddr: start of physical address
 * @size:  size of this contiguous block
 *
 * Caller passes this struct to prepare_dma syscall, which fills in pinned
 * contiguous physical memory blocks. Caller uses DMA_MULTI_PMEM to tell
 * syscall whether it passes in a single struct or an array.
 */
struct dma_pmem {
	uint64_t paddr;
	uint32_t size;
	uint32_t pad;
};

/*
 * Flags for prepare_dma and finish_dma syscalls
 */

/*
 * DMA directions
 */
#define DMA_FLAG_TO_DEVICE		(0x1 << 0)	/* memory to device */
#define DMA_FLAG_FROM_DEVICE		(0x1 << 1)	/* device to memory */
#define DMA_FLAG_BIDIRECTION		(DMA_FLAG_TO_DEVICE | DMA_FLAG_FROM_DEVICE)

/*
 * If DMA_FLAG_MULTI_PMEM is set, caller of prepare_dma must pass an array
 * of dma_pmem struct. The array must be big enough to store all contiguous
 * physical memory blocks to match requested user space memory aperture. For
 * example, when minimum size of contiguous physical memory block is PAGE_SIZE,
 * the array must be big enough to store
 * ((uaddr + size - 1) / PAGE_SIZE - uaddr / PAGE_SIZE + 1) entries.
 * Note: using a smaller dma_pmem array would corrupt current process's memory.
 *
 * If DMA_FLAG_MULTI_PMEM is not set, caller must pass a single dma_pmem struct.
 * prepare_dma will attempt to map user space aperture into one contiguous
 * physical memory block. If user space aperture is bigger than max size of
 * contiguous physical memory block, behavior controlled by DMA_ALLOW_PARTIAL.
 */
#define DMA_FLAG_MULTI_PMEM		(0x1 << 2)

/*
 * If DMA_FLAG_ALLOW_PARTIAL is set, prepare_dma allows partial mapping of user
 * requested aperture, e.g. assuming max size of one contiguous physical block
 * is PAGE_SIZE, when DMA_MULTI_PMEM is not set and [vaddr, vaddr + size) spans
 * on multiple pages, if this bit is set, kernel maps [vaddr, end of first page]
 * with a single dma_pmem struct. If not set, kernel returns ERR_BAD_LEN.
 */
#define DMA_FLAG_ALLOW_PARTIAL	(0x1 << 3)


#endif /* __MM_H */
