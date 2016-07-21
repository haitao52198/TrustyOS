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

/* DEF_SYSCALL(syscall_nr, syscall_name, return type, nr_args, [argument list])
 *
 * Please keep this table sorted by syscall number
 */

DEF_SYSCALL(0x1, write, long, 3, uint32_t fd, void* msg, uint32_t size)
DEF_SYSCALL(0x2, brk, long, 1, uint32_t brk)
DEF_SYSCALL(0x3, exit_group, long, 0)
DEF_SYSCALL(0x4, read, long, 3, uint32_t fd, void* msg, uint32_t size)
DEF_SYSCALL(0x5, ioctl, long, 3, uint32_t fd, uint32_t req, void* buf)
DEF_SYSCALL(0x6, nanosleep, long, 3, uint32_t clock_id, uint32_t flags, uint64_t sleep_time)
DEF_SYSCALL(0x7, gettime, long, 3, uint32_t clock_id, uint32_t flags, int64_t *time)
DEF_SYSCALL(0x8, mmap, long, 4, user_addr_t uaddr, uint32_t size, uint32_t flags, uint32_t handle)
DEF_SYSCALL(0x9, munmap, long, 2, user_addr_t uaddr, uint32_t size)
DEF_SYSCALL(0xa, prepare_dma, long, 4, user_addr_t uaddr, uint32_t size, uint32_t flags, user_addr_t pmem)
DEF_SYSCALL(0xb, finish_dma, long, 3, user_addr_t uaddr, uint32_t size, uint32_t flags)

/* IPC connection establishement syscalls */
DEF_SYSCALL(0x10, port_create, long, 4, const char *path, uint num_recv_bufs, size_t recv_buf_size, uint32_t flags)
DEF_SYSCALL(0x11, connect, long, 2, const char *path, uint flags)
DEF_SYSCALL(0x12, accept, long, 2, uint32_t handle_id, uuid_t *peer_uuid)
DEF_SYSCALL(0x13, close, long, 1, uint32_t handle_id)
DEF_SYSCALL(0x14, set_cookie, long, 2, uint32_t handle, void *cookie)

/* handle polling related syscalls */
DEF_SYSCALL(0x18, wait, long, 3, uint32_t handle_id, uevent_t *event, unsigned long timeout_msecs)
DEF_SYSCALL(0x19, wait_any, long, 2, uevent_t *event, unsigned long timeout_msecs)

/* message send/recv syscalls */
DEF_SYSCALL(0x20, get_msg, long, 2, uint32_t handle, ipc_msg_info_t *msg_info)
DEF_SYSCALL(0x21, read_msg, long, 4, uint32_t handle, uint32_t msg_id, uint32_t offset, ipc_msg_t *msg)
DEF_SYSCALL(0x22, put_msg, long, 2, uint32_t handle, uint32_t msg_id)
DEF_SYSCALL(0x23, send_msg, long, 2, uint32_t handle, ipc_msg_t *msg)
