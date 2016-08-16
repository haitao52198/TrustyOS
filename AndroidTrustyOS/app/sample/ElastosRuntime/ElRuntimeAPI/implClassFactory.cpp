//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include <elastos.h>
#include "trusty_uuid.h"

_ELASTOS_NAMESPACE_USING

/**
 * ClassFactory
 * ELAPI _CObject_AcquireClassFactory()
 *
 * @param appid
 *            id of application
 * @param clsid
 *            id of class
 *
 */
EXTERN_C CARAPI DllGetClassObject(
    uuid_t appid,
    uuid_t clsid,
    PClassObject *ppObj)
{
    return E_CLASS_NOT_AVAILABLE;
}


#if 0
/**
 * CreateObject
 *
 * @param appid
 *            id of application
 * @param clsid
 *            id of class
 *
 */
EXTERN_C CARAPI CreateObject(
    /* out */ IInterface **ppObj)
{
    *ppObj = NULL;
    return E_CLASS_NOT_AVAILABLE;
}
#endif
