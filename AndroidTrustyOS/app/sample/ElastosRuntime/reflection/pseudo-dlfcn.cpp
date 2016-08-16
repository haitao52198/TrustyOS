#include <dlfcn.h>
#include <stddef.h>

void *dlopen(const char *filename, int flag)
{
    return NULL;
}

const char*  dlerror(void)
{
    return NULL;
}

void *dlsym(void *handle, const char *symbol)
{
    return NULL;
}


int dlclose(void *handle)
{
    return 0;
}

