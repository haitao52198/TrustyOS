/** @addtogroup CARRef
  *   @{
  */

//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CARAPI_H__
#define __CARAPI_H__

#include <elatypes.h>

extern "C" {

struct CarMultiQI
{
    const _ELASTOS InterfaceID* mIID;
    PInterface mObject;
    _ELASTOS ECode mEC;
};
typedef struct CarMultiQI CarMultiQI, *PMULTIQI;

#define RGM_INVALID_DOMAIN          ((PRegime)0x0000)
#define RGM_SAME_DOMAIN             ((PRegime)0x0001)
#define RGM_DIFF_DOMAIN             ((PRegime)0x0002)
#define RGM_DEFAULT                 ((PRegime)0x0003)
#define RGM_DIFF_MACHINE            ((PRegime)0x0004)
#define RGM_SAME_PROCESS            RGM_SAME_DOMAIN
#define RGM_DIFF_PROCESS            RGM_DIFF_DOMAIN

#define RGM_MAX_NUMBER              ((PRegime)0xFFFF)
#define IS_INVALID_REGIME(dw)       ((dw == RGM_INVALID_DOMAIN))
#define IS_RGM_NUMBER(dw)           ((!IS_INVALID_REGIME(dw)) && \
                                        ((dw == RGM_SAME_DOMAIN) || \
                                        (dw == RGM_DIFF_DOMAIN) || \
                                        (dw == RGM_DEFAULT) || \
                                        (dw == RGM_DIFF_MACHINE)))

ELAPI ECO_PUBLIC _CObject_CreateInstance(
    /* [in] */ _ELASTOS RClassID rclsid,
    /* [in] */ PRegime regime,
    /* [in] */ _ELASTOS REIID riid,
    /* [out] */ PInterface* object);

ELAPI ECO_PUBLIC _CObject_CreateInstanceEx(
    /* [in] */ _ELASTOS RClassID rclsid,
    /* [in] */ PRegime regime,
    /* [in] */ _ELASTOS UInt32 cmq,
    /* [out] */ PMULTIQI results);

ELAPI ECO_PUBLIC _CObject_AcquireClassFactory(
    /* [in] */ _ELASTOS RClassID rclsid,
    /* [in] */ PRegime regime,
    /* [in] */ _ELASTOS REIID iid,
    /* [out] */ PInterface* object);

ELAPI_(_ELASTOS Boolean) ECO_PUBLIC _CObject_Compare(
    /* [in] */ PInterface objectA,
    /* [in] */ PInterface objectB);

ELAPI ECO_PUBLIC _CObject_AttachAspect(
    /* [in] */ PInterface aggregator,
    /* [in] */ _ELASTOS RClassID aspectClsid);

ELAPI ECO_PUBLIC _CObject_DetachAspect(
    /* [in] */ PInterface aggregator,
    /* [in] */ _ELASTOS RClassID aspectClsid);

ELAPI ECO_PUBLIC _CObject_EnterRegime(
    /* [in] */ PInterface object,
    /* [in] */ PRegime regime);

ELAPI ECO_PUBLIC _CObject_LeaveRegime(
    /* [in] */ PInterface object,
    /* [in] */ PRegime regime);

typedef interface ICallbackSink *PCALLBACKSINK;
typedef interface ICallbackRendezvous* PCallbackRendezvous;

ELAPI ECO_PUBLIC _CObject_AcquireCallbackSink(
    /* [in] */ PInterface object,
    /* [out] */ PCALLBACKSINK* callbackSink);

ELAPI ECO_PUBLIC _CObject_AddCallback(
    /* [in] */ PInterface serverObj,
    /* [in] */ _ELASTOS Int32 event,
    /* [in] */ _ELASTOS EventHandler delegate);

ELAPI ECO_PUBLIC _CObject_RemoveCallback(
    /* [in] */ PInterface serverObj,
    /* [in] */ _ELASTOS Int32 event,
    /* [in] */ _ELASTOS EventHandler delegate);

ELAPI ECO_PUBLIC _CObject_RemoveAllCallbacks(
    /* [in] */ PInterface serverObj);

ELAPI ECO_PUBLIC _CObject_AcquireCallbackRendezvous(
    /* [in] */ PInterface serverObj,
    /* [in] */ _ELASTOS Int32 event,
    /* [out] */ PCallbackRendezvous* callbackRendezvous);

ELAPI_(_ELASTOS Boolean) ECO_PUBLIC _CModule_CanUnloadAllModules();

}
// extern "C"

#endif // __CARAPI_H__
