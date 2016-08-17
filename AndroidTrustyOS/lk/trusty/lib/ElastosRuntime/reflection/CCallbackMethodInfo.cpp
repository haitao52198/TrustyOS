//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CCallbackMethodInfo.h"
#include "CDelegateProxy.h"

CCallbackMethodInfo::CCallbackMethodInfo()
    : mEventNum(0)
    , mMethodDescriptor(NULL)
{}

UInt32 CCallbackMethodInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CCallbackMethodInfo::Release()
{
    return ElLightRefBase::Release();
}

PInterface  CCallbackMethodInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_ICallbackMethodInfo) {
        return (ICallbackMethodInfo *)this;
    }
    else if (riid == EIID_IFunctionInfo) {
        return (IFunctionInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CCallbackMethodInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CCallbackMethodInfo::Init(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ UInt32 eventNum,
    /* [in] */ MethodDescriptor* methodDescriptor,
    /* [in] */ UInt32 uIndex)
{
    mMethodDescriptor = methodDescriptor;
    mEventNum = eventNum;

    AutoPtr<IInterface> obj;
    ECode ec = g_objInfoList.AcquireMethodInfo(clsModule, methodDescriptor,
            uIndex, (IInterface**)&obj);
    mMethodInfo = (CMethodInfo*)IMethodInfo::Probe(obj);
    return ec;
}

ECode CCallbackMethodInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = adjustNameAddr(mMethodInfo->mClsModule->mBase,
            mMethodDescriptor->mName);
    return NOERROR;
}

ECode CCallbackMethodInfo::GetParamCount(
    /* [out] */ Int32* count)
{
    return mMethodInfo->GetParamCount(count);
}

ECode CCallbackMethodInfo::GetAllParamInfos(
    /* [out] */ ArrayOf<IParamInfo *>* paramInfos)
{
    return mMethodInfo->GetAllParamInfos(paramInfos);
}

ECode CCallbackMethodInfo::GetParamInfoByIndex(
    /* [in] */ Int32 index,
    /* [out] */ IParamInfo** paramInfo)
{
    return mMethodInfo->GetParamInfoByIndex(index, paramInfo);
}

ECode CCallbackMethodInfo::GetParamInfoByName(
    /* [in] */ const String& name,
    /* [out] */ IParamInfo** paramInfo)
{
    return mMethodInfo->GetParamInfoByName(name, paramInfo);
}

ECode CCallbackMethodInfo::AddCallback(
    /* [in] */ PInterface server,
    /* [in] */ EventHandler handler)
{
    if (!server) {
        return E_INVALID_ARGUMENT;
    }

    AutoPtr<ICallbackSink> sink;
    ECode ec = _CObject_AcquireCallbackSink(server, (ICallbackSink**)&sink);
    if (FAILED(ec)) return ec;
    return sink->AddCallback(mEventNum, handler);
}

ECode CCallbackMethodInfo::RemoveCallback(
    /* [in] */ PInterface server,
    /* [in] */ EventHandler handler)
{
    if (!server) {
        return E_INVALID_ARGUMENT;
    }

    AutoPtr<ICallbackSink> sink;
    ECode ec = _CObject_AcquireCallbackSink(server, (ICallbackSink**)&sink);
    if (FAILED(ec)) return ec;
    return sink->RemoveCallback(mEventNum, handler);
}

ECode CCallbackMethodInfo::CreateCBArgumentList(
    /* [out] */ ICallbackArgumentList** cbArgumentList)
{
    return mMethodInfo->CreateCBArgumentList(this, cbArgumentList);
}

ECode CCallbackMethodInfo::CreateDelegateProxy(
    /* [in] */ PVoid targetObject,
    /* [in] */ PVoid targetMethod,
    /* [in] */ ICallbackInvocation* callbackInvocation,
    /* [out] */ IDelegateProxy** delegateProxy)
{
    if (!callbackInvocation || !delegateProxy) {
        return E_INVALID_ARGUMENT;
    }

    *delegateProxy = new CDelegateProxy(this, callbackInvocation,
            targetObject, targetMethod);
    if (!*delegateProxy) {
        return E_OUT_OF_MEMORY;
    }

    (*delegateProxy)->AddRef();

    return NOERROR;
}
