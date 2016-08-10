/* dlfcn.h

   Copyright 1998, 1999, 2000, 2001 Red Hat, Inc.

This file is part of Cygwin.

This software is a copyrighted work licensed under the terms of the
Cygwin license.  Please consult the file "CYGWIN_LICENSE" for
details. */

#ifndef _DLFCN_H
#define _DLFCN_H

extern "C" {

/* declarations used for dynamic linking support routines */
extern void*        dlopen(const char*  filename, int flag);
extern int          dlclose(void*  handle);
extern const char*  dlerror(void);
extern void*        dlsym(void*  handle, const char*  symbol);
extern int dlGetClassInfo(void *handle, void **pAddress, int *pSize);

/* following doesn't exist in Win32 API .... */
#define RTLD_DEFAULT  ((void*) 0x10)
#define RTLD_NEXT     ((void*) 0x20)

/* valid values for mode argument to dlopen */
#define RTLD_LAZY	1	/* lazy function call binding */
#define RTLD_NOW	2	/* immediate function call binding */
#define RTLD_GLOBAL	4	/* symbols in this dlopen'ed obj are visible to other dlopen'ed objs */
#define RTLD_LOCAL  8

}

#endif /* _DLFCN_H */
