//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CEnumItemInfo.h"
#include "CEnumInfo.h"

CEnumItemInfo::CEnumItemInfo(
    /* [in] */ IEnumInfo* enumInfo,
    /* [in] */ const String& name,
    /* [in] */ Int32 value)
    : mEnumInfo(enumInfo)
    , mName(name)
    , mValue(value)
{}

CEnumItemInfo::~CEnumItemInfo()
{}

UInt32 CEnumItemInfo::AddRef()
{
    return mEnumInfo->AddRef();
}

UInt32 CEnumItemInfo::Release()
{
    return mEnumInfo->Release();
}

PInterface CEnumItemInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IEnumItemInfo) {
        return (IEnumItemInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CEnumItemInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CEnumItemInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = mName;
    return NOERROR;
}

ECode CEnumItemInfo::GetEnumInfo(
    /* [out] */ IEnumInfo** enumInfo)
{
    if (!enumInfo) {
        return E_INVALID_ARGUMENT;
    }

    *enumInfo = mEnumInfo;
    (*enumInfo)->AddRef();
    return NOERROR;
}

ECode CEnumItemInfo::GetValue(
    /* [out] */ Int32* value)
{
    if (!value) {
        return E_INVALID_ARGUMENT;
    }

    *value = mValue;
    return NOERROR;
}
