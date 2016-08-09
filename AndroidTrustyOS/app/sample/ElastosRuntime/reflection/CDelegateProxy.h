//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CDELEGATEPROXY_H__
#define __CDELEGATEPROXY_H__

#include "refutil.h"

class CDelegateProxy
    : public ElLightRefBase
    , public IDelegateProxy
{
public:
    CDelegateProxy(
        /* [in] */ ICallbackMethodInfo* callbackMethodInfo,
        /* [in] */ ICallbackInvocation* callbackInvocation,
        /* [in] */ PVoid targetObject,
        /* [in] */ PVoid targetMethod);

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI GetCallbackMethodInfo(
        /* [out] */ ICallbackMethodInfo** callbackMethodInfo);

    CARAPI GetTargetObject(
        /* [out] */ PVoid* targetObject);

    CARAPI GetTargetMethod(
        /* [out] */ PVoid* targetMethod);

    CARAPI GetCallbackInvocation(
        /* [out] */ ICallbackInvocation** callbackInvocation);

#ifndef _arm
    CARAPI OnEvent(
        /* [in] */ PInterface server);
#else
    CARAPI OnEvent(
        /* [in] */ PInterface server, ...);
#endif

    CARAPI EventHander(
        /* [in] */ PVoid paramBuf,
        /* [out] */ UInt32* paramBufSize);

    CARAPI GetDelegate(
        /* [out] */ EventHandler* handler);

private:
    AutoPtr<ICallbackMethodInfo>    mCallbackMethodInfo;
    AutoPtr<ICallbackInvocation>    mCallbackInvocation;
    PVoid                           mTargetObject;
    PVoid                           mTargetMethod;
};

#endif // __CDELEGATEPROXY_H__
