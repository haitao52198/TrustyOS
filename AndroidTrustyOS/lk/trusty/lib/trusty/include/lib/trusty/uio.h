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

#ifndef __LIB_TRUSTY_UIO_H
#define __LIB_TRUSTY_UIO_H

#include <sys/types.h>
#include <uthread.h>

typedef struct iovec_kern {
	void		*base;
	size_t		len;
} iovec_kern_t;

typedef struct iovec_user {
	user_addr_t	base;
	uint32_t	len;
} iovec_user_t;


ssize_t membuf_to_kern_iovec(const iovec_kern_t *iov, uint iov_cnt,
                             const uint8_t *buf, size_t len);


ssize_t kern_iovec_to_membuf(uint8_t *buf, size_t len,
                             const iovec_kern_t *iov, uint iov_cnt);

ssize_t membuf_to_user_iovec(user_addr_t iov_uaddr, uint iov_cnt,
                             const uint8_t *buf, size_t cb);

ssize_t user_iovec_to_membuf(uint8_t *buf, size_t len,
                             user_addr_t iov_uaddr, uint iov_cnt);

#endif
