//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================
#include <elastos.h>
#include <stdio.h>
#include <stdlib.h>
// #include <_pubcrt.h>
#include <trusty_app_manifest.h>
#include <xni.h>

#define STATUS_WIN32_ERROR(u) ((ECode)((u)|0x80070000))

#define E_ENTRY_NOT_FOUND         STATUS_WIN32_ERROR(1761)

#define ENABLE_DUMP_CLSID    0    // debug info switch
#if ENABLE_DUMP_CLSID
#include <utils/Log.h>

#define DUMP_CLSID(CLSID, info) \
    do { \
        ALOGD("> INSIDE %s\n", info); \
        ALOGD("======== DUMP_CLSID ========\n"); \
        ALOGD("{%p, %p, %p, {%p, %p, %p, %p, %p, %p, %p, %p} }\n", \
                CLSID.mClsid.mData1, CLSID.mClsid.mData2, CLSID.mClsid.mData3, \
                CLSID.mClsid.mData4[0], CLSID.mClsid.mData4[1], \
                CLSID.mClsid.mData4[2], CLSID.mClsid.mData4[3], \
                CLSID.mClsid.mData4[4], CLSID.mClsid.mData4[5], \
                CLSID.mClsid.mData4[6], CLSID.mClsid.mData4[7]); \
        ALOGD("============================\n"); \
    } while(0);
#else
#define DUMP_CLSID(CLSID, info);
#endif

ELAPI ECO_PUBLIC _CObject_CreateInstance(
    /* [in] */ _ELASTOS RClassID rclsid,
    /* [in] */ PRegime regime,
    /* [in] */ _ELASTOS REIID riid,
    /* [out] */ PInterface* object)
{
    return NOERROR;
}

ELAPI CObject_CreateInstance(
    /* [in] */ REIID appid,
    /* [in] */ REIID clsid,
    /* [out] */ PInterface* obj)
{
    DUMP_CLSID(rclsid, "_CObject_CreateInstance")
    if (NULL == obj)
        return E_INVALID_ARGUMENT;

    PClassObject clsObject = NULL;
    //_CObject_AcquireClassFactory(rclsid, regime, EIID_IClassObject, (PInterface*)&clsObject);
    ECode ec = DllGetClassObject(appid, clsid, &clsObject);
    if (FAILED(ec))
        return ec;

    IInterface* newObject;
    ec = clsObject->CreateObject(NULL, clsid, (IInterface**)&newObject);
    if (FAILED(ec))
        return ec;

    return ec;
}
