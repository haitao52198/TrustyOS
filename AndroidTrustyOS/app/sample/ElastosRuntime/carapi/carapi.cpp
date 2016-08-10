//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================
#include <elapi.h>
#include <locmod.h>
#include <stdio.h>
#include <stdlib.h>
// #include <_pubcrt.h>
#include <utils/Log.h>

#define STATUS_WIN32_ERROR(u) ((ECode)((u)|0x80070000))

#define E_ENTRY_NOT_FOUND         STATUS_WIN32_ERROR(1761)

#define ENABLE_DUMP_CLSID    0    // debug info switch
#if ENABLE_DUMP_CLSID
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

ELAPI _CObject_CreateInstance(
    /* [in] */ RClassID rclsid,
    /* [in] */ PRegime regime,
    /* [in] */ REIID riid,
    /* [out] */ PInterface* obj)
{
    DUMP_CLSID(rclsid, "_CObject_CreateInstance")
    if (NULL == obj || IS_INVALID_REGIME(regime)) return E_INVALID_ARGUMENT;

    CarMultiQI mq = { &riid, NULL, 0 };
    ECode ec;

    if (!IS_RGM_NUMBER(regime)) {
        return regime->CreateObject(rclsid, riid, obj);
    }
    else {
        ec = _CObject_CreateInstanceEx(rclsid, regime, 1, &mq);
    }

    if (SUCCEEDED(ec)) {
        *obj = mq.mObject;
    }

    return ec;
}

ELAPI _CObject_CreateInstanceEx(
    /* [in] */ RClassID rclsid,
    /* [in] */ PRegime regime,
    /* [in] */ UInt32 cmq,
    /* [out] */ PMULTIQI results)
{
    DUMP_CLSID(rclsid, "_CObject_CreateInstanceEx")
    assert(cmq > 0 && results != NULL);
    if (IS_INVALID_REGIME(regime)) return E_INVALID_ARGUMENT;

    PClassObject clsObject = NULL;
    ECode ec = _CObject_AcquireClassFactory(rclsid, regime, EIID_IClassObject, (PInterface*)&clsObject);
    if (FAILED(ec)) return ec;

    results->mEC = clsObject->CreateObject(
            NULL, *(results->mIID), &results->mObject);
    clsObject->Release();
    if (FAILED(results->mEC)) return results->mEC;

    PInterface object = results->mObject;
    for (UInt32 n = 1; n < cmq; n++) {
        results++;
        results->mObject = object->Probe(*(results->mIID));
        if (!results->mObject) ec = E_NO_INTERFACE;
        else results->mObject->AddRef();
        results->mEC = ec;
    }

    return ec;
}

ELAPI _CObject_AcquireClassFactory(
    /* [in] */ RClassID rclsid,
    /* [in] */ PRegime regime,
    /* [in] */ REIID riid,
    /* [out] */ PInterface* object)
{
    DUMP_CLSID(rclsid, "_CObject_AcquireClassFactory")
    if (NULL == object || IS_INVALID_REGIME(regime)) return E_INVALID_ARGUMENT;

    if (regime == RGM_SAME_DOMAIN) {
        return AcquireClassObjectFromLocalModule(rclsid, riid, object);
    }

    return E_NOT_IMPLEMENTED;
}

ELAPI_(Boolean) _CObject_Compare(
    /* [in] */ PInterface objectA,
    /* [in] */ PInterface objectB)
{
    objectA = objectA->Probe(EIID_IInterface);
    if (!objectA) {
        assert(0 && "pObjectA query IInterface failed!");
        return FALSE;
    }
    objectB = objectB->Probe(EIID_IInterface);
    if (!objectB) {
        assert(0 && "pObjectB query IInterface failed!");
        return FALSE;
    }

    return (objectA == objectB);
}

ELAPI _CObject_AttachAspect(
    /* [in] */ PInterface aggregator,
    /* [in] */ RClassID aspectClsid)
{
    IObject* object = (IObject*)aggregator->Probe(EIID_IObject);
    assert(object);

    IAspect* aspect = NULL;
    ECode ec = _CObject_CreateInstance(aspectClsid,
                  RGM_SAME_DOMAIN,
                  EIID_IAspect,
                  (PInterface*)&aspect);
    if (FAILED(ec)) return ec;
    ec = aspect->AspectAggregate(AggrType_Aggregate, object);
    return ec;
}

ELAPI _CObject_DetachAspect(
    /* [in] */ PInterface aggregator,
    /* [in] */ RClassID aspectClsid)
{
    IObject* object = (IObject*)aggregator->Probe(EIID_IObject);
    if (object == NULL) return E_NO_INTERFACE;

    return object->Aggregate(AggrType_Unaggregate,
                    (PInterface)&aspectClsid);
}

ELAPI _CObject_AddCallback(
    /* [in] */ PInterface serverObj,
    /* [in] */ Int32 dwEvent,
    /* [in] */ EventHandler delegate)
{
    ICallbackSink* sink = NULL;
    ECode ec = _CObject_AcquireCallbackSink(serverObj, &sink);
    if (FAILED(ec)) return ec;

    ec = sink->AddCallback(dwEvent, delegate);
    sink->Release();
    return ec;
}

ELAPI _CObject_RemoveCallback(
    /* [in] */ PInterface serverObj,
    /* [in] */ Int32 dwEvent,
    /* [in] */ EventHandler delegate)
{
    ICallbackSink* sink = NULL;
    ECode ec = _CObject_AcquireCallbackSink(serverObj, &sink);
    if (FAILED(ec)) return ec;

    ec = sink->RemoveCallback(dwEvent, delegate);
    sink->Release();
    return ec;
}

ELAPI _CObject_RemoveAllCallbacks(
    /* [in] */ PInterface serverObj)
{
    ICallbackSink* sink = NULL;
    ECode ec;
    do {
        ec = _CObject_AcquireCallbackSink(serverObj, &sink);
        if (FAILED(ec)) return ec;
        ec = sink->RemoveAllCallbacks();
        sink->Release();

        serverObj = serverObj->Probe(EIID_SUPER_OBJECT);
    } while (serverObj);

    return ec;
}

ELAPI _CObject_AcquireCallbackRendezvous(
    /* [in] */ PInterface serverObj,
    /* [in] */ Int32 dwEvent,
    /* [in] */ PCallbackRendezvous* callbackRendezvous)
{
    ICallbackSink* sink = NULL;
    ECode ec = _CObject_AcquireCallbackSink(serverObj, &sink);
    if (FAILED(ec)) return ec;

    ICallbackRendezvous* pICallbackRendezvous = NULL;
    ec = sink->AcquireCallbackRendezvous(dwEvent, &pICallbackRendezvous);
    sink->Release();

    *callbackRendezvous = pICallbackRendezvous;

    return ec;
}

ELAPI _CObject_AcquireCallbackSink(
    /* [in] */ PInterface obj,
    /* [out] */ PCALLBACKSINK* sink)
{
    // Query LOCAL ICallbackConnector interface
    ICallbackConnector* connector = (ICallbackConnector *)obj->Probe(EIID_CALLBACK_CONNECTOR);
    if (!connector) return E_NO_INTERFACE;

    return connector->AcquireCallbackSink(sink);
}
