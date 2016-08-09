//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CCppVectorInfo.h"
#include "CObjInfoList.h"
#include "CStructInfo.h"

CCppVectorInfo::CCppVectorInfo(
    /* [in] */ IDataTypeInfo* elementTypeInfo,
    /* [in] */ Int32 length)
{
    mElementTypeInfo = elementTypeInfo;
    elementTypeInfo->GetSize(&mSize);
    mLength = length;
    mSize *= mLength;
}

UInt32 CCppVectorInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CCppVectorInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_DataType);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        g_objInfoList.DetachCppVectorInfo(this);
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_DataType);
    assert(ref >= 0);
    return ref;
}

PInterface CCppVectorInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_ICppVectorInfo) {
        return (ICppVectorInfo *)this;
    }
    else if (riid == EIID_IDataTypeInfo) {
        return (IDataTypeInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CCppVectorInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CCppVectorInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

    ECode ec = mElementTypeInfo->GetName(name);
    if (FAILED(ec)) return ec;

    name->Append("Vector");

    return NOERROR;
}

ECode CCppVectorInfo::GetSize(
    /* [out] */ MemorySize* size)
{
    if (!size) {
        return E_INVALID_ARGUMENT;
    }

    *size = mSize;

    return NOERROR;
}

ECode CCppVectorInfo::GetDataType(
    /* [out] */ CarDataType* dataType)
{
    if (!dataType) {
        return E_INVALID_ARGUMENT;
    }

    *dataType = CarDataType_CppVector;

    return NOERROR;
}

ECode CCppVectorInfo::GetElementTypeInfo(
    /* [out] */ IDataTypeInfo** elementTypeInfo)
{
    *elementTypeInfo = mElementTypeInfo;
    (*elementTypeInfo)->AddRef();

    return NOERROR;
}

ECode CCppVectorInfo::GetLength(
    /* [out] */ Int32* length)
{
    if (!length) {
        return E_INVALID_ARGUMENT;
    }

    *length = mLength;

    return NOERROR;
}

ECode CCppVectorInfo::GetMaxAlignSize(
    /* [out] */ MemorySize* alignSize)
{
    Int32 size = 1;
    CarDataType dataType;
    mElementTypeInfo->GetDataType(&dataType);
    if (dataType == CarDataType_Struct) {
        ((CStructInfo *)mElementTypeInfo.Get())->GetMaxAlignSize(&size);
    }
    else {
        mElementTypeInfo->GetSize(&size);
    }

    if (size > ALIGN_BOUND) size = ALIGN_BOUND;

    *alignSize = size;

    return NOERROR;
}
