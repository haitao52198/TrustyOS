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
    /* [in] */ REIID appid,
    /* [in] */ REIID clsid,
    /* [out] */ PClassObject *ppObj);

#if 0
EXTERN_C CARAPI CreateObjectFunc(
    /* [out] */ PInterface* object);
#endif

ELAPI CObject_CreateInstance(
    /* [in] */ REIID appid,
    /* [in] */ REIID clsid,
    /* [out] */ PInterface* obj);

