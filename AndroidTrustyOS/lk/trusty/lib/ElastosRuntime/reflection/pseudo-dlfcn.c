
//#include <dlfcn.h>

#include <arch.h>
#include <assert.h>
#include <compiler.h>
#include <debug.h>
#include "elf.h"
#include <err.h>
#include <kernel/mutex.h>
#include <kernel/thread.h>
#include <malloc.h>
#include <platform.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <uthread.h>
#include <lk/init.h>

#include <lib/trusty/trusty_app.h>


extern intptr_t __trusty_app_start;
extern intptr_t __trusty_app_end;
extern trusty_app_t *trusty_app_list;
extern u_int trusty_app_count;

void *dlopen(const char *filename, int flag)
{
    trusty_app_t *trusty_app;
    u_int i;

    char *trusty_app_image_start;
    char *trusty_app_image_end;
    u_int trusty_app_image_size;

    trusty_app_image_start = (char *)&__trusty_app_start;
    trusty_app_image_end = (char *)&__trusty_app_end;
    trusty_app_image_size = (trusty_app_image_end - trusty_app_image_start);

    for (i = 0, trusty_app = trusty_app_list; i < trusty_app_count; i++, trusty_app++) {
         if (strcmp(trusty_app->app_name, filename) == 0)
            break;
    }

    if (i < trusty_app_count) {
        return trusty_app;
    }

    return NULL;
}

const char*  dlerror(void)
{
    return NULL;
}

void *dlsym(void *handle, const char *symbol)
{
    if (strcmp(symbol, "g_pDllResource") == 0)
        return handle;

    return NULL;
}


int dlclose(void *handle)
{
    return 0;
}
