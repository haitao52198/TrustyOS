//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CFieldInfo.h"
#include "CObjInfoList.h"

CFieldInfo::CFieldInfo(
    /* [in] */ IStructInfo* structInfo,
    /* [in] */ const String& name,
    /* [in] */ IDataTypeInfo* typeInfo)
    : mStructInfo(structInfo)
    , mName(name)
    , mTypeInfo(typeInfo)
{}

CFieldInfo::~CFieldInfo()
{}

UInt32 CFieldInfo::AddRef()
{
    return mStructInfo->AddRef();
}

UInt32 CFieldInfo::Release()
{
    return mStructInfo->Release();
}

PInterface CFieldInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IFieldInfo) {
        return (IFieldInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CFieldInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CFieldInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = mName;
    return NOERROR;
}

ECode CFieldInfo::GetTypeInfo(
    /* [out] */ IDataTypeInfo** typeInfo)
{
    if (!typeInfo) {
        return E_INVALID_ARGUMENT;
    }

    *typeInfo = mTypeInfo;
    (*typeInfo)->AddRef();
    return NOERROR;
}
