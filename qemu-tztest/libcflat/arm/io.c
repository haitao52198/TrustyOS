/*
 * Each architecture must implement puts() and exit() with the I/O
 * devices exposed from QEMU, e.g. pl011 and virtio-testdev. That's
 * what's done here, along with initialization functions for those
 * devices.
 *
 * Copyright (C) 2014, Red Hat Inc, Andrew Jones <drjones@redhat.com>
 *
 * This work is licensed under the terms of the GNU LGPL, version 2.
 */
#include "libcflat.h"
#include "io.h"
#include "platform.h"

extern void halt(int code);

/*
 * Use this guess for the pl011 base in order to make an attempt at
 * having earlier printf support. We'll overwrite it with the real
 * base address that we read from the device tree later.
 */
static volatile u8 *uart0_base = (u8 *)UART0_BASE;

void puts(const char *s)
{
	while (*s)
		writeb(*s++, uart0_base);
}

void exit(int code)
{
	halt(code);
}
