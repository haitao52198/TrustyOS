//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CIntrinsicInfo.h"
#include "CObjInfoList.h"
#include "refutil.h"

CIntrinsicInfo::CIntrinsicInfo(
    /* [in] */ CarDataType dataType,
    /* [in] */ UInt32 size)
{
    mDataType = dataType;
    mSize = size;
}

UInt32 CIntrinsicInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CIntrinsicInfo::Release()
{
    return ElLightRefBase::Release();
}

PInterface CIntrinsicInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_IDataTypeInfo) {
        return (IDataTypeInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CIntrinsicInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CIntrinsicInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    *name = g_cDataTypeList[mDataType].mName;
    return NOERROR;
}

ECode CIntrinsicInfo::GetSize(
    /* [out] */ MemorySize* size)
{
    if (!mSize) {
        return E_INVALID_OPERATION;
    }

    if (!size) {
        return E_INVALID_ARGUMENT;
    }

    *size = mSize;
    return NOERROR;
}

ECode CIntrinsicInfo::GetDataType(
    /* [out] */ CarDataType* dataType)
{
    if (!dataType) {
        return E_INVALID_ARGUMENT;
    }

    *dataType = mDataType;
    return NOERROR;
}
