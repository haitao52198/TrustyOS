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

#include <err.h>
#include <string.h>
#include <lib/trusty/uio.h>

ssize_t membuf_to_kern_iovec(const iovec_kern_t *iov, uint iov_cnt,
                             const uint8_t *buf, size_t len)
{
	size_t copied = 0;

	if (unlikely(iov_cnt == 0 || len == 0))
		return 0;

	if (unlikely(iov == NULL || buf == NULL))
		return (ssize_t) ERR_INVALID_ARGS;

	for (uint i = 0; i < iov_cnt; i++, iov++) {

		size_t to_copy = len;
		if (to_copy > iov->len)
			to_copy = iov->len;

		if (unlikely(to_copy == 0))
			continue;

		if (unlikely(iov->base == NULL))
			return (ssize_t) ERR_INVALID_ARGS;

		memcpy(iov->base, buf, to_copy);

		copied += to_copy;
		buf    += to_copy;
		len    -= to_copy;

		if (len == 0)
			break;
	}

	return  (ssize_t) copied;
}

ssize_t kern_iovec_to_membuf(uint8_t *buf, size_t len,
                             const iovec_kern_t *iov, uint iov_cnt)
{
	size_t copied = 0;

	if (unlikely(iov_cnt == 0 || len == 0))
		return 0;

	if (unlikely(buf == NULL || iov == NULL))
		return (ssize_t) ERR_INVALID_ARGS;

	for (uint i = 0; i < iov_cnt; i++, iov++) {

		size_t to_copy = len;
		if (to_copy > iov->len)
			to_copy = iov->len;

		if (unlikely(to_copy == 0))
			continue;

		if (unlikely(iov->base == NULL))
			return (ssize_t) ERR_INVALID_ARGS;

		memcpy (buf, iov->base, to_copy);

		copied += to_copy;
		buf    += to_copy;
		len    -= to_copy;

		if (len == 0)
			break;
	}

	return (ssize_t) copied;
}

ssize_t membuf_to_user_iovec(user_addr_t iov_uaddr, uint iov_cnt,
                             const uint8_t *buf, size_t len)
{
	status_t ret;
	size_t copied = 0;
	iovec_user_t uiov;

	if (unlikely(iov_cnt == 0 || len == 0))
		return 0;

	if (unlikely(buf == NULL))
		return (ssize_t) ERR_INVALID_ARGS;

	/* for each iov entry */
	for (uint i = 0; i < iov_cnt; i++) {

		/* copy user iovec from user space into local buffer */
		ret = copy_from_user(&uiov,
		                     iov_uaddr + i * sizeof(iovec_user_t),
		                     sizeof(iovec_user_t));
		if (unlikely(ret != NO_ERROR))
			return (ssize_t) ret;

		size_t to_copy = len;
		if (to_copy > uiov.len)
			to_copy = uiov.len;

		/* copy data to user space */
		ret = copy_to_user(uiov.base, buf, to_copy);
		if (unlikely(ret != NO_ERROR))
			return (ssize_t) ret;

		copied += to_copy;
		buf    += to_copy;
		len    -= to_copy;

		if (len == 0)
			break;;
	}

	return  (ssize_t) copied;
}

ssize_t user_iovec_to_membuf(uint8_t *buf, size_t len,
                             user_addr_t iov_uaddr, uint iov_cnt)
{
	status_t ret;
	size_t copied = 0;
	iovec_user_t uiov;

	if (unlikely(iov_cnt == 0 || len == 0))
		return 0;

	if (unlikely(buf == NULL))
		return (ssize_t) ERR_INVALID_ARGS;

	for (uint i = 0; i < iov_cnt; i++) {

		/* copy user iovec from user space into local buffer */
		ret = copy_from_user(&uiov,
		                     iov_uaddr + i * sizeof(iovec_user_t),
		                     sizeof(iovec_user_t));
		if (unlikely(ret != NO_ERROR))
			return (ssize_t) ret;

		size_t to_copy = len;
		if (to_copy > uiov.len)
			to_copy = uiov.len;

		/* copy data to user space */
		ret = copy_from_user(buf, uiov.base, to_copy);
		if (unlikely(ret != NO_ERROR))
			return (ssize_t) ret;

		copied += to_copy;
		buf    += to_copy;
		len    -= to_copy;
		if (len == 0)
			break;;
	}

	return (ssize_t) copied;
}
