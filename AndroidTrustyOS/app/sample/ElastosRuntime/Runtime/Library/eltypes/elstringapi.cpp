
#include <sys/types.h>
#include <eltypes.h>
#include <stdlib.h>
#include <ctype.h>

#include "ucase.h"

#define _atoi64 atol

_ELASTOS_NAMESPACE_USING

extern "C"
{
    ECO_PUBLIC SharedBuffer* gElEmptyStringBuf = NULL;
    ECO_PUBLIC char* gElEmptyString = NULL;
}

static void InitString()
{
    SharedBuffer* buf = SharedBuffer::Alloc(1);
    char* str = (char*)buf->GetData();
    *str = 0;
    gElEmptyStringBuf = buf;
    gElEmptyString = str;
}

static void UninitString()
{
    SharedBuffer::GetBufferFromData(gElEmptyString)->Release();
    gElEmptyStringBuf = NULL;
    gElEmptyString = NULL;
}

class LibUtilsFirstStatics
{
public:
    LibUtilsFirstStatics()
    {
        InitString();
    }

    ~LibUtilsFirstStatics()
    {
        UninitString();
    }
};

static LibUtilsFirstStatics gFirstStatics;

extern "C" {

ECO_PUBLIC Char32 __cdecl _String_ToLowerCase(
    /* [in] */ Char32 ch)
{
    return u_tolower(ch);
}

ECO_PUBLIC Char32 __cdecl _String_ToUpperCase(
    /* [in] */ Char32 ch)
{
    return u_toupper(ch);
}

}
