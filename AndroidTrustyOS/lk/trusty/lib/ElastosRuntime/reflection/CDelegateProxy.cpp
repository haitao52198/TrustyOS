//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CDelegateProxy.h"
#include "CCallbackMethodInfo.h"
#include "CCallbackArgumentList.h"

#if defined(_MSVC)
#pragma warning(disable: 4731)
#endif

CDelegateProxy::CDelegateProxy(
    /* [in] */ ICallbackMethodInfo* callbackMethodInfo,
    /* [in] */ ICallbackInvocation* callbackInvocation,
    /* [in] */ PVoid targetObject,
    /* [in] */ PVoid targetMethod)
    : mCallbackMethodInfo(callbackMethodInfo)
    , mCallbackInvocation(callbackInvocation)
    , mTargetObject(targetObject)
    , mTargetMethod(targetMethod)
{}

UInt32 CDelegateProxy::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CDelegateProxy::Release()
{
    return ElLightRefBase::Release();
}

PInterface CDelegateProxy::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IDelegateProxy) {
        return (IDelegateProxy *)this;
    }
    else {
        return NULL;
    }
}

ECode CDelegateProxy::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CDelegateProxy::GetCallbackMethodInfo(
    /* [out] */ ICallbackMethodInfo** callbackMethodInfo)
{
    if (!callbackMethodInfo) {
        return E_INVALID_ARGUMENT;
    }

    *callbackMethodInfo = mCallbackMethodInfo;
    (*callbackMethodInfo)->AddRef();
    return NOERROR;
}

ECode CDelegateProxy::GetTargetObject(
    /* [out] */ PVoid* targetObject)
{
    if (!targetObject) {
        return E_INVALID_ARGUMENT;
    }

    *targetObject = mTargetObject;
    return NOERROR;
}

ECode CDelegateProxy::GetTargetMethod(
    /* [out] */ PVoid* targetMethod)
{
    if (!targetMethod) {
        return E_INVALID_ARGUMENT;
    }

    *targetMethod = mTargetMethod;
    return NOERROR;
}

ECode CDelegateProxy::GetCallbackInvocation(
    /* [out] */ ICallbackInvocation** callbackInvocation)
{
    if (!callbackInvocation) {
        return E_INVALID_ARGUMENT;
    }

    *callbackInvocation = mCallbackInvocation;
    (*callbackInvocation)->AddRef();
    return NOERROR;
}

ECode CDelegateProxy::GetDelegate(
    /* [out] */ EventHandler* handler)
{
    if (!handler) {
        return E_INVALID_ARGUMENT;
    }

#ifndef _arm
    ECode (__stdcall CDelegateProxy::*CBFunc)(PInterface) =
        &CDelegateProxy::OnEvent;
#else
    ECode (__stdcall CDelegateProxy::*CBFunc)(PInterface, ...) =
        &CDelegateProxy::OnEvent;
#endif

    *handler = EventHandler::Make((void *)this, *(void**)&CBFunc);

    return NOERROR;
}

ECode CDelegateProxy::EventHander(
    /* [in] */ PVoid paramBuf,
    /* [out] */ UInt32* paramBufSize)
{
    AutoPtr<ICallbackArgumentList> obj;
    ECode ec = ((CCallbackMethodInfo *)mCallbackMethodInfo.Get())->CreateCBArgumentList(
            (ICallbackArgumentList**)&obj);
    if (FAILED(ec)) {
        return ec;
    }

    CCallbackArgumentList* cbArgumentList = (CCallbackArgumentList*)obj.Get();
    memcpy(cbArgumentList->mParamBuf, paramBuf, cbArgumentList->mParamBufSize);

    if (paramBufSize) {
        *paramBufSize = cbArgumentList->mParamBufSize - 4;
    }

    mCallbackInvocation->Invoke(mTargetObject, mTargetMethod, cbArgumentList);

    return NOERROR;
}

#ifdef _mips
ECode CDelegateProxy::OnEvent(PInterface server)
{
    ASM("break 0;");
    return NOERROR;
}
#elif _x86
ECode CDelegateProxy::OnEvent(PInterface server)
{
    UInt32 moveSize;
    ECode ec = EventHander(&server, &moveSize);

    if (moveSize) {
#if _GNUC
        __asm__(
            "pushl  %%ecx\n"
            "pushl  %%edx\n"
            "pushl  %%esi\n"
            "pushl  %%edi\n"

            "movl   %0, %%edx\n"

            "movl   %%ebp, %%ecx\n"  //get the number need to move
            "subl   %%esp, %%ecx\n"
            "addl   $8,  %%ecx\n"
            "shrl   $2, %%ecx\n"

            "movl   %%ebp, %%esi\n"  //set the esi point to the return address
            "addl   $4, %%esi\n"
            "movl   %%esi, %%edi\n"
            "addl   %%edx, %%edi\n"

            "std\n"             //set direction flag
            "rep movsd\n"
            "cld\n"               //clear direction flag

            "addl   %%edx, %%esp\n"  //adjuct the ebp
            "addl   %%edx, %%ebp\n"  //adjuct the ebp

            "popl   %%edi\n"
            "popl   %%esi\n"
            "popl   %%edx\n"
            "popl   %%ecx\n"
            :
            : "m"(moveSize)
        );
#else //_MSC_VER
        __asm {
            push  ecx
            push  edx
            push  esi
            push  edi

            mov   edx, moveSize

            mov   ecx, ebp
            sub   ecx, esp
            add   ecx, 8
            shr   ecx, 2

            mov   esi, ebp
            add   esi, 4
            mov   edi, esi
            add   edi, edx

            std
            rep movsd
            cld

            add   esp, edx
            add   ebp, edx

            pop   edi
            pop   esi
            pop   edx
            pop   ecx
        }
#endif
    }

    return ec;
}
#elif _arm
ECode CDelegateProxy::OnEvent(PInterface server,...)
{
    return EventHander(&server, NULL);
}
#endif
