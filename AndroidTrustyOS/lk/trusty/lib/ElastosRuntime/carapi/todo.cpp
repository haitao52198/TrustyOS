//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include <elastos.h>
#include "trusty_uuid.h"

_ELASTOS_NAMESPACE_USING

EXTERN_C CARAPI DllGetClassObject(
    uuid_t appid, uuid_t clsid, IInterface **ppObj)
{
    return E_CLASS_NOT_AVAILABLE;
}
