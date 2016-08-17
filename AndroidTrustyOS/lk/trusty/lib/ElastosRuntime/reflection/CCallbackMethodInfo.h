//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#ifndef __CCALLBACKMETHODINFO_H__
#define __CCALLBACKMETHODINFO_H__

#include "CMethodInfo.h"

class CCallbackMethodInfo
    : public ElLightRefBase
    , public ICallbackMethodInfo
{
public:
    CCallbackMethodInfo();

    CARAPI_(PInterface) Probe(
        /* [in] */ REIID riid);

    CARAPI_(UInt32) AddRef();

    CARAPI_(UInt32) Release();

    CARAPI GetInterfaceID(
        /* [in] */ IInterface* object,
        /* [out] */ InterfaceID* iid);

    CARAPI Init(
        /* [in] */ CClsModule* clsModule,
        /* [in] */ UInt32 eventNum,
        /* [in] */ MethodDescriptor* methodDescriptor,
        /* [in] */ UInt32 index);

    CARAPI GetName(
        /* [out] */ String* name);

    CARAPI GetParamCount(
        /* [out] */ Int32* count);

    CARAPI GetAllParamInfos(
        /* [out] */ ArrayOf<IParamInfo *>* paramInfos);

    CARAPI GetParamInfoByIndex(
        /* [in] */ Int32 index,
        /* [out] */ IParamInfo** paramInfo);

    CARAPI GetParamInfoByName(
        /* [in] */ const String& name,
        /* [out] */ IParamInfo** paramInfo);

    CARAPI AddCallback(
        /* [in] */ PInterface server,
        /* [in] */ EventHandler handler);

    CARAPI RemoveCallback(
        /* [in] */ PInterface server,
        /* [in] */ EventHandler handler);

    CARAPI CreateDelegateProxy(
        /* [in] */ PVoid targetObject,
        /* [in] */ PVoid targetMethod,
        /* [in] */ ICallbackInvocation* callbackInvocation,
        /* [out] */ IDelegateProxy** delegateProxy);

    CARAPI CreateCBArgumentList(
        /* [out] */ ICallbackArgumentList** cbArgumentList);

private:
    AutoPtr<CMethodInfo>    mMethodInfo;
    UInt32                  mEventNum;
    MethodDescriptor*       mMethodDescriptor;
};

#endif // __CCALLBACKMETHODINFO_H__
