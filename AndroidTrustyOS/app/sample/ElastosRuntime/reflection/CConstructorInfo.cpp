//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CConstructorInfo.h"
#include <elautoptr.h>

UInt32 CConstructorInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CConstructorInfo::Release()
{
    return ElLightRefBase::Release();
}

PInterface CConstructorInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IConstructorInfo) {
        return (IConstructorInfo *)this;
    }
    else if (riid == EIID_IFunctionInfo) {
        return (IFunctionInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CConstructorInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CConstructorInfo::Init(
    /* [in] */ CClsModule* clsModule,
    /* [in] */ MethodDescriptor* methodDescriptor,
    /* [in] */ UInt32 index,
    /* [in] */ ClassID* clsId)
{
    mOutParamIndex = methodDescriptor->mParamCount - 1;

    mInstClsId.mUunm = mUrn2;
    mClsId.mUunm = mUrn;

    mClsId.mClsid =  clsId->mClsid;
    strcpy(mClsId.mUunm, clsId->mUunm);

    AutoPtr<IInterface> obj;
    mMethodInfo = NULL;
    ECode ec = g_objInfoList.AcquireMethodInfo(clsModule, methodDescriptor,
            index, (IInterface**)&obj);
    mMethodInfo = (CMethodInfo*)obj.Get();
    return ec;
}

ECode CConstructorInfo::GetName(
    /* [out] */ String* name)
{
    return mMethodInfo->GetName(name);
}

ECode CConstructorInfo::GetAnnotation(
    /* [out] */ String* annotation)
{
    if (annotation == NULL) {
        return E_INVALID_ARGUMENT;
    }

    return mMethodInfo->GetAnnotation(annotation);
}

ECode CConstructorInfo::GetParamCount(
    /* [out] */ Int32* count)
{
    ECode ec = mMethodInfo->GetParamCount(count);
    if (ec == NOERROR && *count) {
        *count -= 1;
    }
    return ec;
}

ECode CConstructorInfo::GetAllParamInfos(
    /* [out] */ ArrayOf<IParamInfo *>* paramInfos)
{
    return mMethodInfo->GetAllParamInfos(paramInfos);
}

ECode CConstructorInfo::GetParamInfoByIndex(
    /* [in] */ Int32 index,
    /* [out] */ IParamInfo** paramInfo)
{
    if (!paramInfo || index < 0) {
        return E_INVALID_ARGUMENT;
    }

    if (index >= mOutParamIndex) {
        return E_DOES_NOT_EXIST;
    }

    return mMethodInfo->GetParamInfoByIndex(index, paramInfo);
}

ECode CConstructorInfo::GetParamInfoByName(
    /* [in] */ const String& name,
    /* [out] */ IParamInfo** paramInfo)
{
    return mMethodInfo->GetParamInfoByName(name, paramInfo);
}

ECode CConstructorInfo::CreateArgumentList(
    /* [out] */ IArgumentList** argumentList)
{
    Int32 count = 0;
    ECode ec = GetParamCount(&count);
    if (FAILED(ec)) return ec;

    if (!count) {
        return E_INVALID_OPERATION;
    }

    return mMethodInfo->CreateFunctionArgumentList(this, FALSE, argumentList);
}

ECode CConstructorInfo::CreateObjInRgm(
    /* [in] */ PRegime rgm,
    /* [in] */ IArgumentList* inArgumentList,
    /* [out] */ PInterface* object)
{
    AutoPtr<IInterface> clsObject;
    Int32 count = 0;
    ECode ec = GetParamCount(&count);
    if (FAILED(ec)) return ec;

    AutoPtr<IArgumentList> argumentList = inArgumentList;
    if (argumentList == NULL) {
        if (count) {
            return E_INVALID_ARGUMENT;
        }
        else {
            ec = mMethodInfo->CreateFunctionArgumentList(this, FALSE, (IArgumentList**)&argumentList);
            if (FAILED(ec)) return ec;
        }
    }

    ec = _CObject_AcquireClassFactory(mInstClsId, rgm, EIID_IClassObject, (IInterface**)&clsObject);
    if (FAILED(ec)) return ec;
    ec = argumentList->SetOutputArgumentOfObjectPtrPtr(mOutParamIndex, object);
    if (SUCCEEDED(ec)) {
        ec = mMethodInfo->Invoke(clsObject, argumentList);
    }

    return ec;
}

ECode CConstructorInfo::CreateObject(
    /* [in] */ IArgumentList* argumentList,
    /* [out] */ PInterface* object)
{
    if (!object) {
        return E_INVALID_ARGUMENT;
    }

    return CreateObjInRgm(RGM_SAME_DOMAIN, argumentList, object);
}

ECode CConstructorInfo::CreateObjectInRegime(
    /* [in] */ PRegime rgm,
    /* [in] */ IArgumentList* argumentList,
    /* [out] */ PInterface* object)
{
    if (IS_INVALID_REGIME(rgm) || !object) {
        return E_INVALID_ARGUMENT;
    }

    return CreateObjInRgm(rgm, argumentList, object);
}
