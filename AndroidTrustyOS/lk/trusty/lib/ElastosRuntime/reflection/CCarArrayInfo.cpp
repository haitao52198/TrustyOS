//==========================================================================
// Copyright (c) 2000-2008,  Elastos, Inc.  All Rights Reserved.
//==========================================================================

#include "CCarArrayInfo.h"
#include "CVariableOfCarArray.h"
#include "CObjInfoList.h"
#include "CStructInfo.h"

CCarArrayInfo::CCarArrayInfo(
    /* [in] */ CarDataType quintetType,
    /* [in] */ IDataTypeInfo* elementTypeInfo,
    /* [in] */ CarDataType dataType)
    : mElementTypeInfo(elementTypeInfo)
    , mElementDataType(dataType)
    , mQuintetType(quintetType)
{}

UInt32 CCarArrayInfo::AddRef()
{
    return ElLightRefBase::AddRef();
}

UInt32 CCarArrayInfo::Release()
{
    g_objInfoList.LockHashTable(EntryType_DataType);
    Int32 ref = atomic_dec(&mRef);

    if (0 == ref) {
        g_objInfoList.DetachCarArrayInfo(this);
        delete this;
    }
    g_objInfoList.UnlockHashTable(EntryType_DataType);
    assert(ref >= 0);
    return ref;
}

PInterface CCarArrayInfo::Probe(
    /* [in] */ REIID riid)
{
    if (riid == EIID_IInterface) {
        return (PInterface)this;
    }
    else if (riid == EIID_ICarArrayInfo) {
        return (ICarArrayInfo *)this;
    }
    else if (riid == EIID_IDataTypeInfo) {
        return (IDataTypeInfo *)this;
    }
    else {
        return NULL;
    }
}

ECode CCarArrayInfo::GetInterfaceID(
    /* [in] */ IInterface* object,
    /* [out] */ InterfaceID* iid)
{
    return E_NOT_IMPLEMENTED;
}

ECode CCarArrayInfo::GetName(
    /* [out] */ String* name)
{
    if (name == NULL) {
        return E_INVALID_ARGUMENT;
    }

//    if (mElementDataType == CarDataType_LocalType) {
//        name->Copy(g_cDataTypeList[CarDataType_LocalType].name);
//    }

    *name = g_cDataTypeList[mQuintetType].mName;

    String elementName;
    ECode ec = mElementTypeInfo->GetName(&elementName);
    if (FAILED(ec)) return ec;

    name->Append("<");
    name->Append(elementName);
    if (mElementDataType == CarDataType_Interface) {
        name->Append("*");
    }
    name->Append(">");

    return NOERROR;
}

ECode CCarArrayInfo::GetSize(
    /* [out] */ MemorySize* size)
{
    return E_INVALID_OPERATION;
}

ECode CCarArrayInfo::GetDataType(
    /* [out] */ CarDataType* dataType)
{
    if (!dataType) {
        return E_INVALID_ARGUMENT;
    }

    *dataType = mQuintetType;
    return NOERROR;
}

ECode CCarArrayInfo::GetElementTypeInfo(
    /* [out] */ IDataTypeInfo** elementTypeInfo)
{
    if (!elementTypeInfo) {
        return E_INVALID_ARGUMENT;
    }

    *elementTypeInfo = mElementTypeInfo;
    (*elementTypeInfo)->AddRef();
    return NOERROR;
}

ECode CCarArrayInfo::CreateVariable(
    /* [in] */ Int32 capacity,
    /* [out] */ IVariableOfCarArray** variable)
{
    if (capacity <= 0 || !variable) {
        return E_INVALID_ARGUMENT;
    }

    Int32 size = 0;
    ECode ec = mElementTypeInfo->GetSize(&size);
    if (FAILED(ec)) return ec;

    Int32 bufSize = capacity * size;
    PCarQuintet carQuintet =
            (PCarQuintet)calloc(sizeof(CarQuintet) + bufSize, sizeof(Byte));
    if (!carQuintet) {
        return E_OUT_OF_MEMORY;
    }

    carQuintet->mFlags = DataTypeToFlag(mElementDataType);
    if (mQuintetType == CarDataType_ArrayOf) {
        carQuintet->mUsed = bufSize;
    }

    carQuintet->mSize = bufSize;
    carQuintet->mBuf = (Byte *)carQuintet + sizeof(CarQuintet);

    CVariableOfCarArray* carArrayBox = new CVariableOfCarArray(this,
            carQuintet, TRUE);
    if (carArrayBox == NULL) {
        free(carQuintet);
        return E_OUT_OF_MEMORY;
    }

    *variable = (IVariableOfCarArray *)carArrayBox;
    (*variable)->AddRef();

    return NOERROR;
}

ECode CCarArrayInfo::CreateVariableBox(
    /* [in] */ PCarQuintet variableDescriptor,
    /* [out] */ IVariableOfCarArray** variable)
{
    if (!variableDescriptor || !variable) {
        return E_INVALID_ARGUMENT;
    }

    Int32 size = 0;
    ECode ec = mElementTypeInfo->GetSize(&size);
    if (FAILED(ec)) return ec;
    if (variableDescriptor->mSize < size) {
        return E_INVALID_ARGUMENT;
    }

    if (!(variableDescriptor->mFlags & DataTypeToFlag(mElementDataType))) {
        return E_INVALID_ARGUMENT;
    }

    CVariableOfCarArray* carArrayBox = new CVariableOfCarArray(this,
            variableDescriptor, FALSE);
    if (carArrayBox == NULL) {
        return E_OUT_OF_MEMORY;
    }

    *variable = (IVariableOfCarArray *)carArrayBox;
    (*variable)->AddRef();

    return NOERROR;
}

ECode CCarArrayInfo::GetMaxAlignSize(
    /* [out] */ MemorySize* alignSize)
{
    Int32 size = 1;

    if (mElementDataType == CarDataType_Struct) {
        ((CStructInfo *)mElementTypeInfo.Get())->GetMaxAlignSize(&size);
    }
    else {
        mElementTypeInfo->GetSize(&size);
    }

    if (size > ALIGN_BOUND) size = ALIGN_BOUND;

    *alignSize = size;

    return NOERROR;
}
