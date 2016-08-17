//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include <stdio.h>
#include <stdlib.h>
//#include <zlib.h>
#include <malloc.h>

#include <clsbase.h>

int CompressCLS(
    /* [in] */ void* dest)
{
    CLSModule* destModule = (CLSModule *)dest;
    CLSModule* srcModule = (CLSModule *)alloca(destModule->mSize);
    if (!srcModule) _ReturnError(CLSError_OutOfMemory);

    memcpy(srcModule, destModule, destModule->mSize);
    int dataSize = srcModule->mSize - sizeof(CLSModule);

    if (compress(
        (Bytef *)destModule + sizeof(CLSModule),
        (uLongf *)&dataSize,
        (Bytef *)srcModule + sizeof(CLSModule),
        (uLongf)dataSize) != Z_OK) {
        _ReturnError(CLSError_Compress);
    }

    _ReturnOK(dataSize + sizeof(CLSModule));
}

int UncompressCLS(
    /* [in] */ const void* src,
    /* [in] */ int size,
    /* [in] */ CLSModule* destModule)
{
    CLSModule* srcModule = (CLSModule *)src;
    int dataSize = srcModule->mSize - sizeof(CLSModule);

    if (uncompress(
        (Bytef *)destModule + sizeof(CLSModule),
        (uLongf *)&dataSize,
        (Bytef *)srcModule + sizeof(CLSModule),
        (uLongf)size - sizeof(CLSModule)) != Z_OK) {
        _ReturnError(CLSError_Uncompress);
    }

    memcpy(destModule, srcModule, sizeof(CLSModule));
    _ReturnOK(dataSize + sizeof(CLSModule));
}
