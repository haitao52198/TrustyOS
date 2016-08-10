//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CLSBASE_H__
#define __CLSBASE_H__

#include <stdint.h>
#include <elatypes.h>
#include <string.h>
#include <clsdef.h>

#if defined(_arm) && defined(__GNUC__) && (__GNUC__ >= 4) || defined(_MSVC)
#define ALIGN_BOUND 8
#else
#define ALIGN_BOUND 4
#endif

inline int RoundUp4(int n)
{
    return ((n) + 4 - 1) & ~(4 - 1);
}

extern int sCLSErrorNumber;
extern char sCLSErrorName[];
extern char sCLSErrorMessage[];

#define _Return(ret) return ret

#define _ReturnOK(ret)                  \
{                                       \
    sCLSErrorNumber = CLS_NoError;      \
    return ret;                         \
}

#define _ReturnError(err)                   \
{                                           \
    sCLSErrorNumber = err;                  \
    strcpy(sCLSErrorName, #err);           \
    return err;                             \
}

extern void ExtraMessage(
    /* [in] */ const char*,
    /* [in] */ const char* s2 = NULL,
    /* [in] */ const char* s3 = NULL,
    /* [in] */ const char* s4 = NULL);

#endif // __CLSBASE_H__
